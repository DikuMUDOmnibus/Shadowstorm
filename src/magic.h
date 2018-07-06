/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/


/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(      spell_calm		);
DECLARE_SPELL_FUN(      spell_cancellation	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(      spell_chain_lightning   );
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_rose	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(      spell_cure_disease	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(      spell_dancing_sword     );
DECLARE_SPELL_FUN(      spell_demonfire		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_good	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(      spell_dispel_good       );
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_armor	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_farsight		);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_fireproof		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_floating_disc	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(      spell_frenzy		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(      spell_haste		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_heat_metal	);
DECLARE_SPELL_FUN(      spell_holy_word		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_energy_spike	);
DECLARE_SPELL_FUN(      spell_mass_healing	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(      spell_mirror_shield     );
DECLARE_SPELL_FUN(	spell_nexus		);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(      spell_plague		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_portal		);
DECLARE_SPELL_FUN(	spell_protection_evil	);
DECLARE_SPELL_FUN(	spell_protection_good	);
DECLARE_SPELL_FUN(	spell_ray_of_truth	);
DECLARE_SPELL_FUN(	spell_recharge		);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_slow		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(      spell_trivia_pill       );
DECLARE_SPELL_FUN(      spell_vicegrip          );
DECLARE_SPELL_FUN(      spell_hurricane         );
DECLARE_SPELL_FUN(      spell_greater_heal      );
DECLARE_SPELL_FUN(      spell_super_heal        );
DECLARE_SPELL_FUN(      spell_ultimate_heal     );
DECLARE_SPELL_FUN(      spell_group_heal        );
DECLARE_SPELL_FUN(      spell_bark_skin         );
DECLARE_SPELL_FUN(      spell_knock             );
DECLARE_SPELL_FUN(      spell_group_defense     );
DECLARE_SPELL_FUN(      spell_wizards_fire      );
DECLARE_SPELL_FUN(      spell_meteor_swarm      );
DECLARE_SPELL_FUN(      spell_group_sanctuary   );
DECLARE_SPELL_FUN(      spell_holyblast         );
DECLARE_SPELL_FUN(      spell_lightningblast    );
DECLARE_SPELL_FUN(      spell_electric_blast    );
DECLARE_SPELL_FUN(      spell_gasblast          );
DECLARE_SPELL_FUN(      spell_iceblast          );
DECLARE_SPELL_FUN(      spell_holy_bolt         );
DECLARE_SPELL_FUN(      spell_acid_bolt         );
DECLARE_SPELL_FUN(      spell_gas_bolt          );
DECLARE_SPELL_FUN(      spell_fire_bolt         );
DECLARE_SPELL_FUN(      spell_ice_bolt          );
DECLARE_SPELL_FUN(      spell_diamond_skin      );
DECLARE_SPELL_FUN(      spell_mud_skin          );
DECLARE_SPELL_FUN(      spell_moss_skin         );
DECLARE_SPELL_FUN(      spell_sate              );
DECLARE_SPELL_FUN(      spell_quench            );
DECLARE_SPELL_FUN(      spell_cone_of_cold      );
DECLARE_SPELL_FUN(      spell_synon_aura        );
DECLARE_SPELL_FUN(      spell_drain_blade       );
DECLARE_SPELL_FUN(      spell_shocking_blade    );
DECLARE_SPELL_FUN(      spell_flame_blade       );
DECLARE_SPELL_FUN(      spell_frost_blade       );
DECLARE_SPELL_FUN(      spell_sharp_blade       );
DECLARE_SPELL_FUN(      spell_vorpal_blade      );
DECLARE_SPELL_FUN(      spell_summon_ggolem     );
DECLARE_SPELL_FUN(      spell_fire_and_ice      );
DECLARE_SPELL_FUN(      spell_preserve          );
DECLARE_SPELL_FUN(      spell_sunbolt           );
DECLARE_SPELL_FUN(      spell_heroes_feast      );
DECLARE_SPELL_FUN(      spell_mind_shatter      );
DECLARE_SPELL_FUN(      spell_wizards_firestorm );
DECLARE_SPELL_FUN(      spell_prismatic_spray   );
DECLARE_SPELL_FUN(      spell_revitalize        );
DECLARE_SPELL_FUN(      spell_sturning          );
DECLARE_SPELL_FUN(      spell_steel_skin        );
DECLARE_SPELL_FUN(      spell_tsunami           );    
DECLARE_SPELL_FUN(      spell_shadow_shield     );
DECLARE_SPELL_FUN(      spell_weapon_bless      );
DECLARE_SPELL_FUN(      spell_dark_blessing     );
DECLARE_SPELL_FUN(      spell_divine_protection );
DECLARE_SPELL_FUN(      spell_moonblade         );
DECLARE_SPELL_FUN(      spell_tornado           );
DECLARE_SPELL_FUN(      spell_wizards_fury      );
DECLARE_SPELL_FUN(      spell_glacier           );
DECLARE_SPELL_FUN(      spell_thunderstorm      );
DECLARE_SPELL_FUN(      spell_mystic_armor      );
DECLARE_SPELL_FUN(      spell_desert_fist       );
DECLARE_SPELL_FUN(      spell_resilience        );
DECLARE_SPELL_FUN(      spell_magic_resistance  );
DECLARE_SPELL_FUN(      spell_sandstorm         );
DECLARE_SPELL_FUN(      spell_fire_blade        );
DECLARE_SPELL_FUN(      spell_call_wolf         );
DECLARE_SPELL_FUN(      spell_random            );
DECLARE_SPELL_FUN(      spell_banshee_scream    );
DECLARE_SPELL_FUN(      spell_resurrect         );
DECLARE_SPELL_FUN(      spell_raise_skeleton    );
DECLARE_SPELL_FUN(      spell_iron_golem        );
DECLARE_SPELL_FUN(      spell_stone_golem       );
DECLARE_SPELL_FUN(      spell_adamantite_golem  );
DECLARE_SPELL_FUN(      spell_raise_dead        );
DECLARE_SPELL_FUN(      spell_magic_plate       );
DECLARE_SPELL_FUN(      spell_decay_corpse      );
DECLARE_SPELL_FUN(      spell_create_mummy      );
DECLARE_SPELL_FUN(      spell_call_slayer       );
DECLARE_SPELL_FUN(      spell_lesser_golem      );
DECLARE_SPELL_FUN(      spell_forceshield	);
DECLARE_SPELL_FUN(      spell_staticshield	);
DECLARE_SPELL_FUN(      spell_flameshield	);
DECLARE_SPELL_FUN(      spell_powerstorm        );
DECLARE_SPELL_FUN(      spell_animal_instinct   );
DECLARE_SPELL_FUN(      spell_earthdrain        );
DECLARE_SPELL_FUN(      spell_chaos_flare       );
DECLARE_SPELL_FUN(      spell_mantle            );
DECLARE_SPELL_FUN(      spell_arcane_magic      );
DECLARE_SPELL_FUN(      spell_animate           );
DECLARE_SPELL_FUN(      spell_essence_of_mist   );
DECLARE_SPELL_FUN(      spell_essence_of_magic  );
DECLARE_SPELL_FUN(      spell_essence_of_life   );
DECLARE_SPELL_FUN(      spell_lifeforce         );
DECLARE_SPELL_FUN(      spell_silver_fire       );
DECLARE_SPELL_FUN(      spell_soulburn          );
DECLARE_SPELL_FUN(      spell_icelance          );
DECLARE_SPELL_FUN(      spell_shield_darkness   );
