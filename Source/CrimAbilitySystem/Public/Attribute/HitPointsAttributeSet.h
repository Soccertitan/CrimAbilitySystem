// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CrimAttributeSetBase.h"
#include "HitPointsAttributeSet.generated.h"

/**
 * Class that defines attributes that are necessary for taking damage.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UHitPointsAttributeSet : public UCrimAttributeSetBase
{
	GENERATED_BODY()

public:

	UHitPointsAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, CurrentPoints);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, MaxPoints);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UHitPointsAttributeSet, Damage);

protected:

	UFUNCTION()
	void OnRep_CurrentPoints(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxPoints(const FGameplayAttributeData& OldValue);

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void ClampAttributes(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void HandleDamage(const FGameplayEffectModCallbackData& Data, float Magnitude);
	virtual void HandleHealing(const FGameplayEffectModCallbackData& Data, float Magnitude);

private:

	/**
	 * The current HitPoints attribute. The value will be capped by the MaxHitPoints attribute.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPoints, Category = "Hit Points Attribute Set", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentPoints;

	/**
	 * The current MaxHitPoints attribute. MaxHitPoints is an attribute since gameplay effects can modify it.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoints, Category = "Hit Points Attribute Set", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPoints;

	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category="Hit Points Attribute Set", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category="Hit Points Attribute Set", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};
