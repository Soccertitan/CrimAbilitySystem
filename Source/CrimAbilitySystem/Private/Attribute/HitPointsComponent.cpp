// Copyright Soccertitan 2025


#include "Attribute/HitPointsComponent.h"

#include "CrimAbilityLogChannels.h"
#include "CrimAbilitySystemComponent.h"
#include "AbilityGameplayTags.h"
#include "GameplayEffectExtension.h"
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
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UHitPointsComponent, DeathState, Params);
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
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHitPointsAttributeSet::GetCurrentPointsAttribute()).AddUObject(this, &UHitPointsComponent::OnHitPointsUpdated);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHitPointsAttributeSet::GetMaxPointsAttribute()).AddUObject(this, &UHitPointsComponent::OnMaxHitPointsUpdated);
	
	UE_LOG(LogCrimAbilitySystem, Log, TEXT("HitPointsComponent: has been initialized for owner [%s]."), *GetNameSafe(Owner));
}

void UHitPointsComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHitPointsAttributeSet::GetCurrentPointsAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHitPointsAttributeSet::GetMaxPointsAttribute()).RemoveAll(this);
		AbilitySystemComponent = nullptr;
	}
}

float UHitPointsComponent::GetHitPoints() const
{
	return AbilitySystemComponent ? AbilitySystemComponent->GetNumericAttribute(UHitPointsAttributeSet::GetCurrentPointsAttribute()) : 0;
}

float UHitPointsComponent::GetMaxHitPoints() const
{
	return AbilitySystemComponent ? AbilitySystemComponent->GetNumericAttribute(UHitPointsAttributeSet::GetMaxPointsAttribute()) : 0;
}

float UHitPointsComponent::GetHitPointsNormalized() const
{
	if (AbilitySystemComponent)
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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DeathState, this);
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

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DeathState, this);
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

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DeathState, this);
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

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DeathState, this);
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

void UHitPointsComponent::OnHitPointsUpdated(const FOnAttributeChangeData& Data)
{
	AActor* Instigator = nullptr;
	const FGameplayEffectContext* Spec = nullptr;
	if (Data.GEModData)
	{
		Instigator = Data.GEModData->EffectSpec.GetEffectContext().Get()->GetInstigator();
		Spec = Data.GEModData->EffectSpec.GetEffectContext().Get();
	}
	OnHitPointsUpdatedDelegate.Broadcast(this, Data.OldValue, Data.NewValue, Instigator);
	
	if (Data.NewValue <= 0.f && Data.OldValue > 0.f)
	{
		// I just died!
		AActor* OriginalInstigator = Spec ? Spec->GetOriginalInstigator() : nullptr;
		const FGameplayEffectSpec& EffectSpec = Data.GEModData->EffectSpec;
		const float Magnitude = Data.GEModData->EvaluatedData.Magnitude;
		OnOutOfHitPoints(OriginalInstigator, EffectSpec, Magnitude);
	}
	
	if (Data.OldValue <= 0.f && Data.NewValue > 0.f)
	{
		// I am alive now.
		AActor* OriginalInstigator = Spec ? Spec->GetOriginalInstigator() : nullptr;
		const FGameplayEffectSpec& EffectSpec = Data.GEModData->EffectSpec;
		const float Magnitude = Data.GEModData->EvaluatedData.Magnitude;
		// const float Magnitude = Data.GEModData ? Data.GEModData->EvaluatedData.Magnitude : FMath::Abs(Data.NewValue - Data.OldValue);
		OnHitPointsUpdatedFromZero(OriginalInstigator, EffectSpec, Magnitude);
	}
}

void UHitPointsComponent::OnMaxHitPointsUpdated(const FOnAttributeChangeData& Data)
{
	AActor* Instigator = nullptr;
	if (Data.GEModData)
	{
		Instigator = Data.GEModData->EffectSpec.GetEffectContext().Get()->GetInstigator();
	}
	OnMaxHitPointsUpdatedDelegate.Broadcast(this, Data.OldValue, Data.NewValue, Instigator);
}

void UHitPointsComponent::OnOutOfHitPoints(AActor* Instigator, const FGameplayEffectSpec& EffectSpec, float Magnitude)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		FGameplayEventData Payload;
		Payload.EventTag = FAbilityGameplayTags::Get().Ability_GameplayEvent_Death;
		Payload.Instigator = Instigator;
		Payload.Target = AbilitySystemComponent->GetOwnerActor();
		Payload.OptionalObject = EffectSpec.Def;
		Payload.ContextHandle = EffectSpec.GetEffectContext();
		Payload.InstigatorTags = *EffectSpec.CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags = *EffectSpec.CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude = Magnitude;

		FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
#endif // #if WITH_SERVER_CODE
}

void UHitPointsComponent::OnHitPointsUpdatedFromZero(AActor* Instigator, const FGameplayEffectSpec& EffectSpec, float Magnitude)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent)
	{
		// Send the "GameplayEvent.Resurrection" gameplay event through the owner's ability system.  This can be used to trigger a resurrection gameplay ability.
		FGameplayEventData Payload;
		Payload.EventTag = FAbilityGameplayTags::Get().Ability_GameplayEvent_Revive;
		Payload.Instigator = Instigator;
		Payload.Target = AbilitySystemComponent->GetOwnerActor();
		Payload.OptionalObject = EffectSpec.Def;
		Payload.ContextHandle = EffectSpec.GetEffectContext();
		Payload.InstigatorTags = *EffectSpec.CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags = *EffectSpec.CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude = Magnitude;

		FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
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


