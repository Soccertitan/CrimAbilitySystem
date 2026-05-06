// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CrimAbilitySystemInterface.generated.h"

class UCrimAbilitySystemComponent;
// This class does not need to be modified.
UINTERFACE()
class UCrimAbilitySystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Add this interface to ActorComponents to respond when the AbilitySystemComponent is initialized.
 */
class CRIMABILITYSYSTEM_API ICrimAbilitySystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Called in InitActorInfo for ActorComponents on the AvatarActor and OwnerActor.
	 * @param AbilitySystemComponent The ability system component to initialize with.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCrimAbilitySystem(UCrimAbilitySystemComponent* AbilitySystemComponent);
};
