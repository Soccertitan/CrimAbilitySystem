// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputTypes.h"
#include "Engine/DataAsset.h"
#include "AbilityInputSet.generated.h"

/**
 * Contains an array of AbilityInputParams mapped to slots.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UAbilityInputSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "EditorDisplayName"))
	TArray<FAbilityInputParams> Items;
};
