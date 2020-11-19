object_draft_schematic_armor_armor_appearance_mandalorian_rebel_bracer_r = object_draft_schematic_armor_shared_armor_appearance_mandalorian_rebel_bracer_r:new {
    templateType = DRAFTSCHEMATIC,

    customObjectName = "Mandalorian Rebel Right Bracer",

    craftingToolTab = 2, -- (See DraftSchematicObjectTemplate.h)
    complexity = 45, 
    size = 4, 

    xpType = "crafting_clothing_armor", 
    xp = 250, 

    assemblySkill = "armor_assembly", 
    experimentingSkill = "armor_experimentation", 
    customizationSkill = "armor_customization", 

    customizationOptions = {2},
    customizationStringNames = {"/private/index_color_1"},
    customizationDefaults = {0},

    ingredientTemplateNames = {"craft_clothing_ingredients_n", "craft_clothing_ingredients_n"},
    ingredientTitleNames = {"auxilary_coverage", "body"},
    ingredientSlotType = {0, 0},
    resourceTypes = {"hide", "metal"},
    resourceQuantities = {150, 150},
    contribution = {100, 100},


    targetTemplate = "object/tangible/wearables/armor/mandalorian_rebel/armor_mandalorian_rebel_bracer_r.iff",

    additionalTemplates = {}
}

ObjectTemplates:addTemplate(object_draft_schematic_armor_armor_appearance_mandalorian_rebel_bracer_r, "object/draft_schematic/armor/armor_appearance_mandalorian_rebel_bracer_r.iff")
