// Copyright Soccertitan 2025


#include "Attribute/CrimAttributeSetBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CrimAbilitySystemComponent.h"

UCrimAttributeSetBase::UCrimAttributeSetBase()
{
}

UCrimAbilitySystemComponent* UCrimAttributeSetBase::GetCrimAbilitySystemComponent() const
{
	return Cast<UCrimAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

void UCrimAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttributes(Attribute, NewValue);
}

void UCrimAttributeSetBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttributes(Attribute, NewValue);
}
