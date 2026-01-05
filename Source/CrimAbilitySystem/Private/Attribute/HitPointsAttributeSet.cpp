// Copyright Soccertitan 2025


#include "Attribute/HitPointsAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "CrimAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UHitPointsAttributeSet::UHitPointsAttributeSet()
	: CurrentPoints(100.f), MaxPoints(100.f)
{
}

void UHitPointsAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHitPointsAttributeSet, CurrentPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHitPointsAttributeSet, MaxPoints, COND_None, REPNOTIFY_Always);
}

void UHitPointsAttributeSet::OnRep_CurrentPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHitPointsAttributeSet, CurrentPoints, OldValue);

	const float CurrentHitPoints = GetCurrentPoints();
	const float EstimatedMagnitude = CurrentHitPoints - OldValue.GetCurrentValue();

	OnCurrentPointsUpdatedDelegate.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHitPoints);

	if (!bOutOfCurrentPoints && CurrentHitPoints <= 0.0f)
	{
		OnOutOfCurrentPointsDelegate.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHitPoints);
	}
	else if (bOutOfCurrentPoints && CurrentHitPoints > 0.0f)
	{
		OnCurrentPointsUpdatedFromZeroDelegate.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHitPoints);
	}

	bOutOfCurrentPoints = CurrentHitPoints <= 0.0f;
}

void UHitPointsAttributeSet::OnRep_MaxPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHitPointsAttributeSet, MaxPoints, OldValue);

	OnMaxPointsUpdatedDelegate.Broadcast(nullptr, nullptr, nullptr, GetMaxPoints() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxPoints());
}

bool UHitPointsAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	CurrentPointsBeforeAttributeChange = GetCurrentPoints();
	MaxPointsBeforeAttributeChange = GetMaxPoints();

	return true;
}

void UHitPointsAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetCurrentPointsAttribute())
	{
		SetCurrentPoints(FMath::Clamp(GetCurrentPoints(), 0.f, GetMaxPoints()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxPointsAttribute())
	{
		SetCurrentPoints(FMath::Clamp(GetCurrentPoints(), 0.f, GetMaxPoints()));
		OnMaxPointsUpdatedDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxPointsBeforeAttributeChange, GetMaxPoints());
	}
	else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0.f)
		{
			HandleDamage(Data, Data.EvaluatedData.Magnitude);
		}
		else
		{
			HandleHealing(Data, Data.EvaluatedData.Magnitude);
		}
		SetDamage(0.f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0.f)
		{
			HandleHealing(Data, Data.EvaluatedData.Magnitude);
		}
		else
		{
			HandleDamage(Data, Data.EvaluatedData.Magnitude);
		}
		SetHealing(0.f);
	}

	// If HitPoints has actually changed, activate callback
	if (GetCurrentPoints() != CurrentPointsBeforeAttributeChange)
	{
		OnCurrentPointsUpdatedDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, CurrentPointsBeforeAttributeChange, GetCurrentPoints());
	}
	
	if (GetCurrentPoints() <= 0.0f && !bOutOfCurrentPoints)
	{
		OnOutOfCurrentPointsDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, CurrentPointsBeforeAttributeChange, GetCurrentPoints());
	}
	else if (GetCurrentPoints() > 0.0f && bOutOfCurrentPoints)
	{
		OnCurrentPointsUpdatedFromZeroDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, CurrentPointsBeforeAttributeChange, GetCurrentPoints());
	}

	bOutOfCurrentPoints = GetCurrentPoints() <= 0.0f;
}

void UHitPointsAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UHitPointsAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHitPointsAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxPointsAttribute())
	{
		// Make sure current HitPoints is not greater than the new MaxHitPoints.
		if (GetCurrentPoints() > NewValue)
		{
			UCrimAbilitySystemComponent* CrimASC = GetCrimAbilitySystemComponent();
			check(CrimASC);

			CrimASC->ApplyModToAttribute(GetCurrentPointsAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
}

void UHitPointsAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetCurrentPointsAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoints());
	}
	else if (Attribute == GetMaxPointsAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UHitPointsAttributeSet::HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude)
{
	const float LocalDamage = FMath::Abs(Magnitude);

	SetCurrentPoints(FMath::Clamp(GetCurrentPoints() - LocalDamage, 0.f, GetMaxPoints()));
}

void UHitPointsAttributeSet::HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude)
{
	const float LocalHealing = FMath::Abs(Magnitude);

	SetCurrentPoints(FMath::Clamp(GetCurrentPoints() + LocalHealing, 0.f, GetMaxPoints()));
}
