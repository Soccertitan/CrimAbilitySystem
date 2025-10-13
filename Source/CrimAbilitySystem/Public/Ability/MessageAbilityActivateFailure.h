// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "MessageAbilityActivateFailure.generated.h"

USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FMessageAbilityActivateFailureMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	// User facing text message.
	UPROPERTY(BlueprintReadWrite)
	FText FailureReason;
};
