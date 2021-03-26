// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "opencv2/core.hpp"
#include "GameFramework/Actor.h"
#include "OpenCV_Reader.generated.h"

UCLASS()
class UE4_OPENCV_API AOpenCV_Reader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpenCV_Reader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
