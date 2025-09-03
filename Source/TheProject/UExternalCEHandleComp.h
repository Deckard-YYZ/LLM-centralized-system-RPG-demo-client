#pragma once

#include "CoreMinimal.h"
#include "CEListenerRunnable.h"
#include "Components/ActorComponent.h"
#include "FParsedCEInfoCPP.h"
#include "UExternalCEHandleComp.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UExternalCEHandleComp: public UActorComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void ForwardInstructionToGameMode(const FParsedCEInfoCPP& Instruction);

	UFUNCTION(BlueprintCallable)
	void SendMessageToServer(const FToSendPreCEInfo& Info);

private:
	CEListenerRunnable* ListenerRunnable;
	FRunnableThread* Thread;

	void HandleMessage(const FString& Message);
};
