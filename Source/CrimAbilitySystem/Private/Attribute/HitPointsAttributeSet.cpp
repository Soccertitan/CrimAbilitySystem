// Copyright Soccertitan 2025


#include "Attribute/HitPointsAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "CrimAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UHitPointsAttributeSet::UHitPointsAttributeSet()
{
	CurrentPoints = 1.f;
	MaxPoints = 1.f;
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
}

void UHitPointsAttributeSet::OnRep_MaxPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHitPointsAttributeSet, MaxPoints, OldValue);
}

void UHitPointsAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentPointsAttribute())
	{
		SetCurrentPoints(FMath::Clamp(GetCurrentPoints(), 0.f, GetMaxPoints()));
		return;
	}
	
	if (Data.EvaluatedData.Attribute == GetMaxPointsAttribute())
	{
		SetCurrentPoints(FMath::Clamp(GetCurrentPoints(), 0.f, GetMaxPoints()));
		return;
	}
	
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
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
		return;
	}
	
	if (Data.EvaluatedData.Attribute == GetHealingAttribute())
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
		return;
	}
}

void UHitPointsAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxPointsAttribute())
	{
		// Make sure current HitPoints is not greater than the new MaxHitPoints.
		if (GetCurrentPoints() > NewValue)
		{
			GetOwningAbilitySystemComponentChecked()->SetNumericAttributeBase(GetCurrentPointsAttribute(), NewValue);
		}
	}
}

void UHitPointsAttributeSet::ClampAttributes(const FGameplayAttribute& Attribute, float& NewValue) const
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
