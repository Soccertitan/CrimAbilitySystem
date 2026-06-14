// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputSlot.h"
#include "Engine/DataAsset.h"
#include "AbilityInputSet.generated.h"

class UAbilityInput;

USTRUCT()
struct FAbilityInputItem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FAbilityInputSlot Slot;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAbilityInput> AbilityInput;
	
	UPROPERTY(VisibleAnywhere, meta = (EditCondition=false, EditConditionHides))
	FString EditorDisplayName;
	
	void PostSerialize(const FArchive& Ar);
};
template<>
struct TStructOpsTypeTraits<FAbilityInputItem> : public TStructOpsTypeTraitsBase2<FAbilityInputItem>
{
	enum
	{
		WithPostSerialize = true,
   };
};

/**
 * Contains an array of AbilityInputItems mapped to slots.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UAbilityInputSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "EditorDisplayName"))
	TArray<FAbilityInputItem> Items;
};
