// Copyright Soccertitan

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
 * 
 */
class CRIMABILITYSYSTEM_API ICrimAbilitySystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * When InitActorInfo is called, all components on the AvatarActor and OwnerActor can respond when the ASC is configured.
	 * @param NewAbilitySystemComponent The ability system component to initialize with.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void InitializeWithAbilitySystem(UCrimAbilitySystemComponent* NewAbilitySystemComponent);
};
