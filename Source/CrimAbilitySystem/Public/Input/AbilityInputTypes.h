// Copyright Soccertitan 2026

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputSlot.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "AbilityInputTypes.generated.h"


USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilityInputParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityInputSlot Slot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass;
	
	bool IsValid() const;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, meta = (EditCondition=false, EditConditionHides))
	FString EditorDisplayName;
#endif
	
#if WITH_EDITOR
	void PostSerialize(const FArchive& Ar);
#endif
};
template<>
struct TStructOpsTypeTraits<FAbilityInputParams> : public TStructOpsTypeTraitsBase2<FAbilityInputParams>
{
	enum
	{
		WithPostSerialize = true,
   };
};
