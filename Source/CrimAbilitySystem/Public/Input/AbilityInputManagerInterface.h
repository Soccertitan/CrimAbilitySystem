// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AbilityInputManagerInterface.generated.h"

class UAbilityInputManagerComponent;
// This class does not need to be modified.
UINTERFACE()
class UAbilityInputManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CRIMABILITYSYSTEM_API IAbilityInputManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/**
	 * Returns the AbilityInputManagerComponent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Crim Ability System|Input")
	UAbilityInputManagerComponent* GetAbilityInputManagerComponent() const;
};
