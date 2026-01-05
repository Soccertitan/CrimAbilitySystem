// Copyright Soccertitan 2025


#include "Attribute/HitPointsComponent.h"

#include "CrimAbilityLogChannels.h"
#include "CrimAbilitySystemComponent.h"
#include "AbilityGameplayTags.h"
#include "Attribute/HitPointsAttributeSet.h"
#include "Net/UnrealNetwork.h"


UHitPointsComponent::UHitPointsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHitPointsComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHitPointsComponent, DeathState);
}

UHitPointsComponent* UHitPointsComponent::FindHitPointsComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UHitPointsComponent>() : nullptr;
}

void UHitPointsComponent::InitializeWithAbilitySystem_Implementation(UCrimAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogCrimAbilitySystem, Log, TEXT("HitPointsComponent: Hit Points component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogCrimAbilitySystem, Error, TEXT("HitPointsComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HitPointsSet = AbilitySystemComponent->GetSet<UHitPointsAttributeSet>();
	if (!HitPointsSet)
	{
		UE_LOG(LogCrimAbilitySystem, Warning, TEXT("HitPointsComponent: Cannot initialize health component for owner [%s] with NULL HitPointsAttributeSet on the ability system."), *GetNameSafe(Owner));
		AbilitySystemComponent = nullptr;
		return;
	}

	// Register to listen for attribute changes.
	HitPointsSet->OnCurrentPointsUpdatedDelegate.AddUObject(this, &ThisClass::OnHitPointsUpdated);
	HitPointsSet->OnMaxPointsUpdatedDelegate.AddUObject(this, &ThisClass::OnMaxHitPointsUpdated);
	HitPointsSet->OnOutOfCurrentPointsDelegate.AddUObject(this, &ThisClass::OnOutOfHitPoints);
	HitPointsSet->OnCurrentPointsUpdatedFromZeroDelegate.AddUObject(this, &ThisClass::OnHitPointsUpdatedFromZero);

	OnHitPointsUpdatedDelegate.Broadcast(this, HitPointsSet->GetCurrentPoints(), HitPointsSet->GetCurrentPoints(), nullptr);
	OnMaxHitPointsUpdatedDelegate.Broadcast(this, HitPointsSet->GetMaxPoints(), HitPointsSet->GetMaxPoints(), nullptr);
	
	UE_LOG(LogCrimAbilitySystem, Log, TEXT("HitPointsComponent: has been initialized for owner [%s]."), *GetNameSafe(Owner));
}

void UHitPointsComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HitPointsSet)
	{
		HitPointsSet->OnCurrentPointsUpdatedDelegate.RemoveAll(this);
		HitPointsSet->OnMaxPointsUpdatedDelegate.RemoveAll(this);
		HitPointsSet->OnOutOfCurrentPointsDelegate.RemoveAll(this);
	}

	HitPointsSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float UHitPointsComponent::GetHitPoints() const
{
	return HitPointsSet ? HitPointsSet->GetCurrentPoints() : 0;
}

float UHitPointsComponent::GetMaxHitPoints() const
{
	return HitPointsSet ? HitPointsSet->GetMaxPoints() : 0;
}

float UHitPointsComponent::GetHitPointsNormalized() const
{
	if (HitPointsSet)
	{
		const float HitPoints = GetHitPoints();
		const float MaxHitPoints = GetMaxHitPoints();

		return MaxHitPoints > 0.0f ? HitPoints / MaxHitPoints : 0.0f;
	}
	return 0.0f;
}

bool UHitPointsComponent::IsDeadOrDying() const
{
	return DeathState == EDeathState::DeathStarted || DeathState == EDeathState::DeathFinished;
}

bool UHitPointsComponent::IsAliveOrReviving() const
{
	return DeathState == EDeathState::Alive || DeathState == EDeathState::ReviveStarted;
}

void UHitPointsComponent::StartDeath()
{
	DeathState = EDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dying, 1);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Reviving, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dead, 0);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStartedDelegate.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UHitPointsComponent::FinishDeath()
{
	DeathState = EDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dead, 1);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Reviving, 0);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinishedDelegate.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UHitPointsComponent::StartRevive()
{
	DeathState = EDeathState::ReviveStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Reviving, 1);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dead, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dying, 0);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnReviveStartedDelegate.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UHitPointsComponent::FinishRevive()
{
	DeathState = EDeathState::Alive;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dead, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Reviving, 0);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnReviveFinishedDelegate.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UHitPointsComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	Super::OnUnregister();
}

void UHitPointsComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Dead, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(FAbilityGameplayTags::Get().Gameplay_State_Death_Reviving, 0);
	}
}

void UHitPointsComponent::OnHitPointsUpdated(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHitPointsUpdatedDelegate.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UHitPointsComponent::OnMaxHitPointsUpdated(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHitPointsUpdatedDelegate.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UHitPointsComponent::OnOutOfHitPoints(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = FAbilityGameplayTags::Get().Ability_GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}
#endif // #if WITH_SERVER_CODE
}

void UHitPointsComponent::OnHitPointsUpdatedFromZero(AActor* RestoreInstigator, AActor* RestoreCauser,
	const FGameplayEffectSpec* RestoreEffectSpec, float RestoreMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && RestoreEffectSpec)
	{
		// Send the "GameplayEvent.Resurrection" gameplay event through the owner's ability system.  This can be used to trigger a resurrection gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = FAbilityGameplayTags::Get().Ability_GameplayEvent_Revive;
			Payload.Instigator = RestoreInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = RestoreEffectSpec->Def;
			Payload.ContextHandle = RestoreEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *RestoreEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *RestoreEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = RestoreMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}
#endif // #if WITH_SERVER_CODE
}

void UHitPointsComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	const EDeathState NewDeathState = DeathState;

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState == EDeathState::Alive)
	{
		if (NewDeathState == EDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == EDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogCrimAbilitySystem, Error, TEXT("HitPointsComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EDeathState::DeathStarted)
	{
		if (NewDeathState == EDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogCrimAbilitySystem, Error, TEXT("HitPointsComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EDeathState::DeathFinished)
	{
		if (NewDeathState == EDeathState::ReviveStarted)
		{
			StartRevive();
		}
		else if (NewDeathState == EDeathState::Alive)
		{
			StartRevive();
			FinishRevive();
		}
		else
		{
			UE_LOG(LogCrimAbilitySystem, Error, TEXT("HitPointsComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EDeathState::ReviveStarted)
	{
		if (NewDeathState == EDeathState::Alive)
		{
			FinishRevive();
		}
		else
		{
			UE_LOG(LogCrimAbilitySystem, Error, TEXT("HitPointsComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("HitPointsComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
}


