// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_GameplayEvent.generated.h"

/**
 * Sends out a Gameplay Event Tag when this notify is reached. 
 */
UCLASS(MinimalAPI, DisplayName = "GameplayEvent")
class UAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAnimNotify_GameplayEvent();
	virtual void BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override;
#endif

protected:
	UPROPERTY(EditAnywhere, Category=GameplayEvent)
	FGameplayTag EventTag;
};
