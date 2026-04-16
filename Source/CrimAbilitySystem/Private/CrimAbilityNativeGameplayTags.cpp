// Copyright Soccertitan 2025


#include "CrimAbilityNativeGameplayTags.h"


namespace CrimAbility::NativeGameplayTag
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability, "Ability", "Root Tag for Ability related events and states.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability Failed due to activation group requirements.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate due to death.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_GameplayEvent_Death, "Ability.GameplayEvent.Death", "Triggers death gameplay abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_GameplayEvent_Revive, "Ability.GameplayEvent.Revive", "Triggers revive gameplay abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_InputBlocked, "Ability.InputBlocked", "Blocks abilities from being activated by player input.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_State, "Ability.State", "Root tag for various states the ASC/Ability can have.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_State_Death, "Ability.State.Death", "Is in a death state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_State_Death_Dead, "Ability.State.Death.Dead", "Finished the death state and is now dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_State_Death_Dying, "Ability.State.Death.Dying", "In the process of entering the dead state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_State_Death_Reviving, "Ability.State.Death.Reviving", "In the process of exiting the death state.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input, "Input", "Root Tag for input events.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message, "Message", "Root Tag to send messages via Gameplay Message Subsystem");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Ability_Activate_Failure, "Message.Ability.Activate.Failure", "A generic tag indicating the ability failed to activate. Used in the gameplay messaging subsystem.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cooldown, "SetByCaller.Cooldown", "The tag used to set a cooldown duration for a GE.");
}
