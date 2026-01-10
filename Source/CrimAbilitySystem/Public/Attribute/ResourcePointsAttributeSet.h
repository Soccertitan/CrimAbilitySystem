// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CrimAttributeSet.h"
#include "ResourcePointsAttributeSet.generated.h"

/**
 * Attributes that are necessary for activating abilities. Like mana, action points, etc.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UResourcePointsAttributeSet : public UCrimAttributeSet
{
	GENERATED_BODY()

public:
	UResourcePointsAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UResourcePointsAttributeSet, CurrentPoints);
	ATTRIBUTE_ACCESSORS(UResourcePointsAttributeSet, MaxPoints);
	ATTRIBUTE_ACCESSORS(UResourcePointsAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UResourcePointsAttributeSet, Damage);

protected:

	UFUNCTION()
	void OnRep_CurrentPoints(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxPoints(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void ClampAttributes(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude);
	virtual void HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude);

private:

	/**
	 * The current TacticalPoints attribute. The value will be capped by the MaxTacticalPoints attribute.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPoints, Category = "Resource Attribute Set", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentPoints;

	/**
	 * The current MaxTacticalPoints attribute. MaxTacticalPoints is an attribute since gameplay effects can modify it.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoints, Category = "Resource Attribute Set", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPoints;

	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +CurrentPoints
	UPROPERTY(BlueprintReadOnly, Category="Resource Attribute Set", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -CurrentPoints
	UPROPERTY(BlueprintReadOnly, Category="Resource Attribute Set", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};
