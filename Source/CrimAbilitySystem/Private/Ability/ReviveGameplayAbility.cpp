// Copyright Soccertitan


#include "Ability/ReviveGameplayAbility.h"

#include "AbilityGameplayTags.h"
#include "CrimAbilityLogChannels.h"
#include "CrimAbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Attribute/HitPointsComponent.h"

namespace ReviveTag
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(Ability_GameplayEvent_Revive, "Ability.GameplayEvent.Revive")
}

UReviveGameplayAbility::UReviveGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ReviveTag::Ability_GameplayEvent_Revive;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UReviveGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	UCrimAbilitySystemComponent* AbilitySystemComponent = CastChecked<UCrimAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	// FGameplayTagContainer AbilityTypesToIgnore;
	// AbilityTypesToIgnore.AddTag(FAbilityGameplayTags::Ability_Behavior_SurvivesDeath);

	// Cancel all abilities and block others from starting.
	// CrimASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	SetCanBeCanceled(false);

	if (!ChangeActivationGroup(EAbilityActivationGroup::Exclusive_Blocking))
	{
		UE_LOG(LogCrimAbilitySystem, Error, TEXT("UReviveGameplayAbility::ActivateAbility: Ability [%s] failed to change activation group to blocking."), *GetName());
	}

	if (bAutoStartRevive)
	{
		StartRevive();
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UReviveGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);

	// Always try to finish the resurrection when the ability ends in case the ability doesn't.
	// This won't do anything if the resurrection hasn't been started.
	FinishRevive();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UReviveGameplayAbility::StartRevive()
{
	if (UHitPointsComponent* HitPointsComponent = UHitPointsComponent::FindHitPointsComponent(GetAvatarActorFromActorInfo()))
	{
		if (HitPointsComponent->IsDeadOrDying())
		{
			HitPointsComponent->StartRevive();
		}
	}
}

void UReviveGameplayAbility::FinishRevive()
{
	if (UHitPointsComponent* HitPointsComponent = UHitPointsComponent::FindHitPointsComponent(GetAvatarActorFromActorInfo()))
	{
		if (HitPointsComponent->GetDeathState() == EDeathState::ReviveStarted)
		{
			HitPointsComponent->FinishRevive();
		}
	}
}
