// Copyright Soccertitan


#include "Attribute/HitPointsAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "CrimAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UHitPointsAttributeSet::UHitPointsAttributeSet()
	: HitPoints(100.f), MaxHitPoints(100.f)
{
}

void UHitPointsAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHitPointsAttributeSet, HitPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHitPointsAttributeSet, MaxHitPoints, COND_None, REPNOTIFY_Always);
}

void UHitPointsAttributeSet::OnRep_HitPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHitPointsAttributeSet, HitPoints, OldValue);

	const float CurrentHitPoints = GetHitPoints();
	const float EstimatedMagnitude = CurrentHitPoints - OldValue.GetCurrentValue();

	OnHitPointsUpdatedDelegate.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHitPoints);

	if (!bOutOfHitPoints && CurrentHitPoints <= 0.0f)
	{
		OnOutOfHitPointsDelegate.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHitPoints);
	}
	else if (bOutOfHitPoints && CurrentHitPoints > 0.0f)
	{
		OnHitPointsUpdatedFromZeroDelegate.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHitPoints);
	}

	bOutOfHitPoints = CurrentHitPoints <= 0.0f;
}

void UHitPointsAttributeSet::OnRep_MaxHitPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHitPointsAttributeSet, MaxHitPoints, OldValue);

	OnMaxHitPointsUpdatedDelegate.Broadcast(nullptr, nullptr, nullptr, GetMaxHitPoints() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxHitPoints());
}

bool UHitPointsAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	HitPointsBeforeAttributeChange = GetHitPoints();
	MaxHitPointsBeforeAttributeChange = GetMaxHitPoints();

	return true;
}

void UHitPointsAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetHitPointsAttribute())
	{
		SetHitPoints(FMath::Clamp(GetHitPoints(), 0.f, GetMaxHitPoints()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHitPointsAttribute())
	{
		SetHitPoints(FMath::Clamp(GetHitPoints(), 0.f, GetMaxHitPoints()));
		OnMaxHitPointsUpdatedDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHitPointsBeforeAttributeChange, GetMaxHitPoints());
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
	if (GetHitPoints() != HitPointsBeforeAttributeChange)
	{
		OnHitPointsUpdatedDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HitPointsBeforeAttributeChange, GetHitPoints());
	}
	
	if (GetHitPoints() <= 0.0f && !bOutOfHitPoints)
	{
		OnOutOfHitPointsDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HitPointsBeforeAttributeChange, GetHitPoints());
	}
	else if (GetHitPoints() > 0.0f && bOutOfHitPoints)
	{
		OnHitPointsUpdatedFromZeroDelegate.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HitPointsBeforeAttributeChange, GetHitPoints());
	}

	bOutOfHitPoints = GetHitPoints() <= 0.0f;
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

	if (Attribute == GetMaxHitPointsAttribute())
	{
		// Make sure current HitPoints is not greater than the new MaxHitPoints.
		if (GetHitPoints() > NewValue)
		{
			UCrimAbilitySystemComponent* CrimASC = GetCrimAbilitySystemComponent();
			check(CrimASC);

			CrimASC->ApplyModToAttribute(GetHitPointsAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
}

void UHitPointsAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHitPointsAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHitPoints());
	}
	else if (Attribute == GetMaxHitPointsAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UHitPointsAttributeSet::HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude)
{
	const float LocalDamage = FMath::Abs(Magnitude);

	SetHitPoints(FMath::Clamp(GetHitPoints() - LocalDamage, 0.f, GetMaxHitPoints()));
}

void UHitPointsAttributeSet::HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude)
{
	const float LocalHealing = FMath::Abs(Magnitude);

	SetHitPoints(FMath::Clamp(GetHitPoints() + LocalHealing, 0.f, GetMaxHitPoints()));
}
