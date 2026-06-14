// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "AbilityInputSlot.generated.h"


/** 
 * The slot an ability can be placed in.
 */
USTRUCT(BlueprintType)
struct FAbilityInputSlot
{
	GENERATED_BODY()
	
	FAbilityInputSlot(){}
	FAbilityInputSlot(FGameplayTag InInputTag) : InputTag(InInputTag) {}
	FAbilityInputSlot(int32 InSlotIndex) : SlotIndex(InSlotIndex) {}
	
	bool IsValid() const;
	FString GetString() const;
	
	FORCEINLINE bool operator == (FAbilityInputSlot const& Other) const
	{
		return InputTag == Other.InputTag || SlotIndex == Other.SlotIndex;
	}

	FORCEINLINE bool operator != (FAbilityInputSlot const& Other) const
	{
		return InputTag != Other.InputTag || SlotIndex != Other.SlotIndex;
	}
	
	void PostSerialize(const FArchive& Ar);
	
private:
	// The InputTag takes priority over the SlotIndex if it's valid.
	UPROPERTY(EditAnywhere, meta = (Categories = "Input"))
	FGameplayTag InputTag;
	
	// If not using an InputTag to define the slot, an int can be used.
	UPROPERTY(EditAnywhere, meta = (ClampMin = -1, EditCondition=bCanChangeSlotIndex))
	int32 SlotIndex = -1;
	
	bool bCanChangeSlotIndex = true;
};
template<>
struct TStructOpsTypeTraits<FAbilityInputSlot> : public TStructOpsTypeTraitsBase2<FAbilityInputSlot>
{
	enum
	{
		WithPostSerialize = true,
   };
};
