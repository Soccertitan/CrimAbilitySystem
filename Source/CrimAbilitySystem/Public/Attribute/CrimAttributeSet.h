// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "CrimAttributeSet.generated.h"

/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(UCrimHealthSet, Health)
 * will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

class UCrimAbilitySystemComponent;

/**
 * Base class for Attribute Sets. Includes Macros and helper functions.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UCrimAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCrimAttributeSet();

	UCrimAbilitySystemComponent* GetCrimAbilitySystemComponent() const;

protected:
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void ClampAttributes(const FGameplayAttribute& Attribute, float& NewValue) const {}
};
