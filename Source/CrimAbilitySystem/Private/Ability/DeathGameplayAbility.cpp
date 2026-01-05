// Copyright Soccertitan 2025


#include "Ability/DeathGameplayAbility.h"

#include "CrimAbilityLogChannels.h"
#include "CrimAbilitySystemComponent.h"
#include "AbilityGameplayTags.h"
#include "NativeGameplayTags.h"
#include "Attribute/HitPointsComponent.h"

namespace DeathTag
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(Ability_GameplayEvent_Death, "Ability.GameplayEvent.Death")
}

UDeathGameplayAbility::UDeathGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	ActivationPolicy = EAbilityActivationPolicy::OnEvent;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = DeathTag::Ability_GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UDeathGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	UCrimAbilitySystemComponent* CrimASC = CastChecked<UCrimAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	// FGameplayTagContainer AbilityTypesToIgnore;
	// AbilityTypesToIgnore.AddTag(FAbilityGameplayTags::Ability_Behavior_SurvivesDeath);

	// Cancel all abilities and block others from starting.
	// CrimASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	SetCanBeCanceled(false);

	if (bAutoStartDeath)
	{
		StartDeath();
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UDeathGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);

	// Always try to finish the death when the ability ends in case the ability doesn't.
	// This won't do anything if the death hasn't been started.
	FinishDeath();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDeathGameplayAbility::StartDeath()
{
	if (UHitPointsComponent* HitPointsComponent = UHitPointsComponent::FindHitPointsComponent(GetAvatarActorFromActorInfo()))
	{
		if (HitPointsComponent->GetDeathState() == EDeathState::Alive)
		{
			HitPointsComponent->StartDeath();
		}
	}
}

void UDeathGameplayAbility::FinishDeath()
{
	if (UHitPointsComponent* HitPointsComponent = UHitPointsComponent::FindHitPointsComponent(GetAvatarActorFromActorInfo()))
	{
		if (HitPointsComponent->GetDeathState() == EDeathState::DeathStarted)
		{
			HitPointsComponent->FinishDeath();
		}
	}
}
