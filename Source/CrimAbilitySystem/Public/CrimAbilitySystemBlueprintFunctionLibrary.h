// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrimAbilitySystemBlueprintFunctionLibrary.generated.h"

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
};
