// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrimAbilitySystemBlueprintFunctionLibrary.generated.h"

struct FGameplayTagContainer;
enum class EGameplayModEvaluationChannel : uint8;
struct FGameplayAttribute;
class UAbilitySystemComponent;
class UAbilityInputManagerComponent;
class UCrimAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class CRIMABILITYSYSTEM_API UCrimAbilitySystemBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * Searches the passed in actor for an ability system component, will use IAbilitySystemInterface or fall back to a
	 * component search.
	 */
	UFUNCTION(BlueprintPure, meta = (Cateogry = "Crim Ability System", DefaultToSelf = Actor))
	static UCrimAbilitySystemComponent* GetAbilitySystemComponent(AActor* Actor, bool LookForComponent = true);

	UFUNCTION(BlueprintPure, meta = (Cateogry = "Crim Ability System", DefaultToSelf = Actor))
	static UAbilityInputManagerComponent* GetAbilityInputManagerComponent(AActor* Actor, bool LookForComponent = true);
	
	/** Returns the value of Attribute from the ability system component AbilitySystem after evaluating it with source and target tags and up to the channel. bSuccess indicates the success or failure of this operation. */
	UFUNCTION(BlueprintPure, Category = "Ability|Attribute")
	static float EvaluateAttributeValueWithTagsUpToChannel(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, EGameplayModEvaluationChannel Channel, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, bool& bSuccess);
	
	/** Returns the value of Attribute from the ability system component AbilitySystem after evaluating it with source and target tags and up to the channel using the passed in base value instead of the real base value. bSuccess indicates the success or failure of this operation. */
	UFUNCTION(BlueprintPure, Category = "Ability|Attribute")
	static float EvaluateAttributeValueWithTagsUpToChannelAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, EGameplayModEvaluationChannel Channel, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess);
};
