// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "AbilityInput.generated.h"

class UGameplayAbility;
class UCrimAbilitySystemComponent;
struct FGameplayEventData;

/**
 * A data asset that defines an ability to activate with input.
 */
UCLASS(Const)
class CRIMABILITYSYSTEM_API UAbilityInput : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UAbilityInput();
	
	/** The ability to activate. The ability must be activatable via Press/Held input. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSoftClassPtr<UGameplayAbility> GameplayAbility;
	
	/** If true, the AbilityInputManager will retrieve GameplayEventData to pass along to the ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	bool bSendGameplayEventData = false;
	
	/** A generic object for custom data. Typically, holds a DataAsset to describe this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UObject> CustomData;
	
	/** Returns a new GameplayEventData to pass to an ability activation. */
	UFUNCTION(BlueprintNativeEvent)
	FGameplayEventData MakeGameplayEventData(UCrimAbilitySystemComponent* AbilitySystemComponent) const;
	
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
protected:
	FGameplayEventData MakeGameplayEventData_Implementation(UCrimAbilitySystemComponent* AbilitySystemComponent) const;
};
