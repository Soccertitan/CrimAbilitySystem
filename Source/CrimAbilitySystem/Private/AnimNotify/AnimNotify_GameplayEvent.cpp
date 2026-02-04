// Copyright Soccertitan 2025


#include "AnimNotify/AnimNotify_GameplayEvent.h"

#include "Animation/AnimMontage.h"
#include "CrimAbilitySystemBlueprintFunctionLibrary.h"
#include "CrimAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"

UAnimNotify_GameplayEvent::UAnimNotify_GameplayEvent()
{
	bIsNativeBranchingPoint = true;
}

void UAnimNotify_GameplayEvent::BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotify(BranchingPointPayload);
	
	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		AActor* Actor = MeshComp->GetOwner();
		if (UCrimAbilitySystemComponent* ASC = UCrimAbilitySystemBlueprintFunctionLibrary::GetAbilitySystemComponent(Actor))
		{
			ASC->OnAnimNotifyDelegate.Broadcast(EventTag);
		}
	}
}

FString UAnimNotify_GameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.ToString();
}

#if WITH_EDITOR
bool UAnimNotify_GameplayEvent::CanBePlaced(UAnimSequenceBase* Animation) const
{
	return (Animation && Animation->IsA(UAnimMontage::StaticClass()));
}
#endif
