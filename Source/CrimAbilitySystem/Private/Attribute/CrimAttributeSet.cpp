// Copyright Soccertitan 2025


#include "Attribute/CrimAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CrimAbilitySystemComponent.h"

UCrimAttributeSet::UCrimAttributeSet()
{
}

UCrimAbilitySystemComponent* UCrimAttributeSet::GetCrimAbilitySystemComponent() const
{
	return Cast<UCrimAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

void UCrimAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttributes(Attribute, NewValue);
}

void UCrimAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttributes(Attribute, NewValue);
}
