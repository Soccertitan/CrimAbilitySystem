// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CrimAttributeSet.h"
#include "HitPointsAttributeSet.generated.h"

/**
 * Class that defines attributes that are necessary for taking damage.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UHitPointsAttributeSet : public UCrimAttributeSet
{
	GENERATED_BODY()

public:
	UHitPointsAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	ATTRIBUTE_ACCESSORS(ThisClass, CurrentPoints);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxPoints);
	ATTRIBUTE_ACCESSORS(ThisClass, Healing);
	ATTRIBUTE_ACCESSORS(ThisClass, Damage);

protected:
	UFUNCTION()
	void OnRep_CurrentPoints(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxPoints(const FGameplayAttributeData& OldValue);

	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude);
	virtual void HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude);

private:

	/**
	 * The CurrentPoints Attribute is clamped between 0.f and MaxPoints.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPoints, Category = "Attribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentPoints;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoints, Category = "Attribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPoints;

	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +CurrentPoints
	UPROPERTY(BlueprintReadOnly, Category="Attribute", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -CurrentPoints
	UPROPERTY(BlueprintReadOnly, Category="Attribute", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};
