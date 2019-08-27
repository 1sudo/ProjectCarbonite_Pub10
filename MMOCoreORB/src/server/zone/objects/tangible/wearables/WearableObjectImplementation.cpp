/*
 * WearableObjectImplementation.cpp
 *
 *  Created on: 02/08/2009
 *      Author: victor
 */

#include "server/zone/objects/tangible/wearables/WearableObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/manufactureschematic/craftingvalues/CraftingValues.h"
#include "server/zone/objects/manufactureschematic/ManufactureSchematic.h"
#include "server/zone/objects/draftschematic/DraftSchematic.h"
#include "server/zone/objects/tangible/attachment/Attachment.h"
#include "server/zone/managers/skill/SkillModManager.h"

/**
 * Rename for clarity/convenience
 */
typedef VectorMapEntry<String,int> Mod;

/**
 * @inf
 * The ModSortingHelper class inherits from VectorMap<String, int> and is used
 * to provide the correct sorting for a SEA stat mod in a SortedVector.
 * Specifically, it keeps the relative order of same-value mods so that when a
 * mod is applied to a wearable, ties are broke in the order they appear on the
 * SEA.
 * @imp
 * Overwrites the VectorMapEntry<k,v>::compareTo method to get custom sort
 * behavior in sorted containers.
 */
class ModSortingHelper : public Mod {
public:
	ModSortingHelper(): Mod( "", 0) {}
	ModSortingHelper( String name, int value ) : Mod( name, value ) {}

	/**
	 * @inf
	 * Overwrite the compareTo method of VectorMapEntry in order to provide
	 * custom sorting logic.
	 * @imp
	 * Using the default compare method where equality values return 0 will
	 * not maintain the relative order of same-value objects being 'put()'
	 * into a SortedVector. This pastebin (http://pastebin.com/AazeG0Lq)
	 * shows how inserting 4 equal elements, A,B,C,D, into a SortedVector
	 * will result in a vector of {A, C, D, B}. This overload results in a
	 * vector of {A, B, C, D }.
	 */
	int compareTo(const Mod& e) const  {
		// Make copies of this and e to get around getValue not being const
		if(  Mod(*this).getValue() >= Mod(e).getValue() ) {
			return 1;
		} else {
			return -1;
		}
	}
};

void WearableObjectImplementation::initializeTransientMembers() {
	TangibleObjectImplementation::initializeTransientMembers();
	setLoggingName("WearableObject");
}

void WearableObjectImplementation::fillAttributeList(AttributeListMessage* alm,
		CreatureObject* object) {
	TangibleObjectImplementation::fillAttributeList(alm, object);

	if (socketsLeft() > 0)
		alm->insertAttribute("sockets", socketsLeft());

	for(int i = 0; i < wearableSkillMods.size(); ++i) {
		String key = wearableSkillMods.elementAt(i).getKey();
		String statname = "cat_skill_mod_bonus.@stat_n:" + key;
		int value = wearableSkillMods.get(key);

		if (value > 0)
			alm->insertAttribute(statname, value);
	}

	//Anti Decay Kit
	if(hasAntiDecayKit() && !isArmorObject()){
		alm->insertAttribute("@veteran_new:antidecay_examine_title", "@veteran_new:antidecay_examine_text");
	}

}

bool WearableObjectImplementation::hasSeaRemovalTool(CreatureObject* player, bool removeItem) {

	if (player == NULL) {
		return 0;
	}

	uint32 crc;
	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	if (inventory == NULL){
		return false;
	}

	Locker inventoryLocker(inventory);

	for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);

		crc = sceno->getServerObjectCRC();
		if (String::valueOf(crc) == "3905622464") { //Sea Removal Tool

			if (sceno != NULL) {
				if (removeItem) {
					Locker locker(sceno);
					sceno->destroyObjectFromWorld(true);
					sceno->destroyObjectFromDatabase(true);
				}

				return true;
			}
		}
	}

	return 0;
}

void WearableObjectImplementation::updateCraftingValues(CraftingValues* values, bool initialUpdate) {
	/*
	 * Values available:	Range:
	 * sockets				0-0(novice artisan) (Don't use)
	 * hitpoints			1000-1000 (Don't Use)
	 */
	if(initialUpdate) {
		if(values->hasProperty("sockets") && values->getCurrentValue("sockets") >= 0)
			generateSockets(values);
	}
}

void WearableObjectImplementation::generateSockets(CraftingValues* craftingValues) {
	// Moved inits of player and manuSchematic so I can use Player outside of the logic block below
	ManagedReference<CreatureObject*> player = nullptr;
	ManagedReference<ManufactureSchematic*> manuSchematic = nullptr;

	if (socketsGenerated) {
		return;
	}

	int skill = 0;
	int luck = 0;

	if (craftingValues != NULL) {
		manuSchematic = craftingValues->getManufactureSchematic();
		if(manuSchematic != NULL) {
			ManagedReference<DraftSchematic*> draftSchematic = manuSchematic->getDraftSchematic();
			player = manuSchematic->getCrafter().get();

			if (player != NULL && draftSchematic != NULL) {
				String assemblySkill = draftSchematic->getAssemblySkill();
				skill = player->getSkillMod(assemblySkill) * 2.5; // 0 to 250 max
				luck = System::random(player->getSkillMod("luck")
						+ player->getSkillMod("force_luck"));
			}
		}
	}

	// Shifted roll chance from (-250 to 500) -> (100 to 850).
	// This will drastically increase max sockets chance, but not gaurantee it.
	// You will always roll at LEAST 1 socket.
	int random = (System::random(750)) + 100; // -250 to 500

	float roll = System::random(skill + luck + random);

	int generatedCount = int(float(MAXSOCKETS * roll) / float(MAXSOCKETS * 100));

	if (generatedCount > MAXSOCKETS)
		generatedCount = MAXSOCKETS;
	if (generatedCount < 0)
		generatedCount = 0;

	// TODO: remove this backwards compatibility fix at next wipe. Only usedSocketCount variable should be used.
	objectCreatedPreUsedSocketCountFix = false;
	usedSocketCount = 0;

	socketCount = generatedCount;

	socketsGenerated = true;
}

int WearableObjectImplementation::socketsUsed() {
	// TODO: remove this backwards compatibility fix at next wipe. Only usedSocketCount variable should be used.
	if (objectCreatedPreUsedSocketCountFix) {
		return wearableSkillMods.size() - modsNotInSockets;
	} else {
		return usedSocketCount;
	}
}

void WearableObjectImplementation::applyAttachment(CreatureObject* player,
		Attachment* attachment) {

	if (!isASubChildOf(player))
		return;

	if (socketsLeft() > 0) {

		Locker locker(player);

		if (isEquipped()) {
			removeSkillModsFrom(player);
		}

		if (wearableSkillMods.size() < 6) {
			HashTable<String, int>* mods = attachment->getSkillMods();
			HashTableIterator<String, int> iterator = mods->iterator();

			String statName;
			int newValue;

			SortedVector< ModSortingHelper > sortedMods;
			for( int i = 0; i < mods->size(); i++){
				iterator.getNextKeyAndValue(statName, newValue);
				sortedMods.put( ModSortingHelper( statName, newValue));
			}

			// Select the next mod in the SEA, sorted high-to-low. If that skill mod is already on the
			// wearable, with higher or equal value, don't apply and continue. Break once one mod
			// is applied.
			for( int i = 0; i < sortedMods.size(); i++ ) {
				String modName = sortedMods.elementAt(i).getKey();
				int modValue = sortedMods.elementAt(i).getValue();

				int existingValue = -26;
				if(wearableSkillMods.contains(modName))
					existingValue = wearableSkillMods.get(modName);

				if( modValue > existingValue) {
					wearableSkillMods.put( modName, modValue );
					break;
				}
			}
		}

		usedSocketCount++;
		addMagicBit(true);
		Locker clocker(attachment, player);
		attachment->destroyObjectFromWorld(true);
		attachment->destroyObjectFromDatabase(true);

		if (isEquipped()) {
			applySkillModsTo(player);
		}
	}
}

void WearableObjectImplementation::applySkillModsTo(CreatureObject* creature) const {
	if (creature == NULL) {
		return;
	}

	for (int i = 0; i < wearableSkillMods.size(); ++i) {
		String name = wearableSkillMods.elementAt(i).getKey();
		int value = wearableSkillMods.get(name);

		if (!SkillModManager::instance()->isWearableModDisabled(name))
			creature->addSkillMod(SkillModManager::WEARABLE, name, value, true);
	}

	SkillModManager::instance()->verifyWearableSkillMods(creature);
}

void WearableObjectImplementation::removeSkillModsFrom(CreatureObject* creature) {
	if (creature == NULL) {
		return;
	}

	for (int i = 0; i < wearableSkillMods.size(); ++i) {
		String name = wearableSkillMods.elementAt(i).getKey();
		int value = wearableSkillMods.get(name);

		if (!SkillModManager::instance()->isWearableModDisabled(name))
			creature->removeSkillMod(SkillModManager::WEARABLE, name, value, true);
	}

	SkillModManager::instance()->verifyWearableSkillMods(creature);
}

bool WearableObjectImplementation::isEquipped() {
	ManagedReference<SceneObject*> parent = getParent().get();
	if (parent != NULL && parent->isPlayerCreature())
		return true;

	return false;
}

String WearableObjectImplementation::repairAttempt(int repairChance) {

	String message = "@error_message:";

	if(repairChance < 25) {
		message += "sys_repair_failed";
		setMaxCondition(1, true);
		setConditionDamage(0, true);
	} else if(repairChance < 50) {
		message += "sys_repair_imperfect";
		setMaxCondition(getMaxCondition() * .65f, true);
		setConditionDamage(0, true);
	} else if(repairChance < 75) {
		setMaxCondition(getMaxCondition() * .80f, true);
		setConditionDamage(0, true);
		message += "sys_repair_slight";
	} else {
		setMaxCondition(getMaxCondition() * .95f, true);
		setConditionDamage(0, true);
		message += "sys_repair_perfect";
	}

	return message;
}

