// Copyright Soccertitan


#include "Attribute/TacticalPointsAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "CrimAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UTacticalPointsAttributeSet::UTacticalPointsAttributeSet()
	: CurrentPoints(100.f), MaxPoints(100.f)
{
}

void UTacticalPointsAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTacticalPointsAttributeSet, CurrentPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTacticalPointsAttributeSet, MaxPoints, COND_None, REPNOTIFY_Always);
}

void UTacticalPointsAttributeSet::OnRep_CurrentPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTacticalPointsAttributeSet, CurrentPoints, OldValue);
}

void UTacticalPointsAttributeSet::OnRep_MaxPoints(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTacticalPointsAttributeSet, MaxPoints, OldValue);
}

bool UTacticalPointsAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
}

void UTacticalPointsAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetCurrentPointsAttribute())
	{
		SetCurrentPoints(FMath::Clamp(GetCurrentPoints(), 0.f, GetMaxPoints()));
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
}

void UTacticalPointsAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UTacticalPointsAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UTacticalPointsAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxPointsAttribute())
	{
		// Make sure current TacticalPoints is not greater than the new MaxTacticalPoints.
		if (GetCurrentPoints() > NewValue)
		{
			UCrimAbilitySystemComponent* CrimASC = GetCrimAbilitySystemComponent();
			check(CrimASC);

			CrimASC->ApplyModToAttribute(GetCurrentPointsAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
}

void UTacticalPointsAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetCurrentPointsAttribute())
	{
		// Do not allow CurrentPoints to go negative or above MaxPoints.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoints());
	}
	else if (Attribute == GetMaxPointsAttribute())
	{
		// Do not allow MaxPoints to drop below 0.
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UTacticalPointsAttributeSet::HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude)
{
	const float LocalDamage = FMath::Abs(Magnitude);

	SetCurrentPoints(FMath::Clamp(GetCurrentPoints() - LocalDamage, 0.f, GetMaxPoints()));
}

void UTacticalPointsAttributeSet::HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude)
{
	const float LocalHealing = FMath::Abs(Magnitude);

	SetCurrentPoints(FMath::Clamp(GetCurrentPoints() + LocalHealing, 0.f, GetMaxPoints()));
}
