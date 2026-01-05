// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayTagEvent.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayTagEventSignature, FGameplayTag, Tag);

/**
 * Listens for changes to gameplay tags on the AbilitySystemComponent.
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class CRIMABILITYSYSTEM_API UGameplayTagEvent : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FGameplayTagEventSignature OnTagAdded;

	UPROPERTY(BlueprintAssignable)
	FGameplayTagEventSignature OnTagRemoved;

	// Listens for FGameplayTags added and removed.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UGameplayTagEvent* ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent* InAbilitySystemComponent, FGameplayTagContainer InTags);

	/**
	 * You must call this function manually when you want the AsyncTask to end.
	 * For UMG Widgets, you would call it in the Widget's Destruct event.
	 */
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FGameplayTagContainer Tags;

	virtual void TagChanged(const FGameplayTag Tag, int32 NewCount);
};
