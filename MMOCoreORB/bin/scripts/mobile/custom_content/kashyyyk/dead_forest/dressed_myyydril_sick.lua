dressed_myyydril_sick = Creature:new {
	customName = "a sickened Myyydril refugee",
	socialGroup = "townsperson",
	faction = "townsperson",
	level = 4,
	chanceHit = 0.24,
	damageMin = 40,
	damageMax = 45,
	baseXp = 62,
	baseHAM = 113,
	baseHAMmax = 118,
	armor = 0,
	resists = {0,0,0,0,0,0,0,-1,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = NONE,
	creatureBitmask = NONE,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_myyydril_sick_01.iff",
		"object/mobile/dressed_myyydril_sick_02.iff",
		"object/mobile/dressed_myyydril_sick_03.iff",
		"object/mobile/dressed_myyydril_sick_04.iff",
		"object/mobile/dressed_myyydril_sick_05.iff",
		"object/mobile/dressed_myyydril_sick_06.iff"
	},
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
	}
}

CreatureTemplates:addCreatureTemplate(dressed_myyydril_sick, "dressed_myyydril_sick")
