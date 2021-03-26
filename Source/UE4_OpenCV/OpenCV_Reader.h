#pragma once
#include "CoreMinimal.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "MediaTexture.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"
#include "OpenCV_Reader.generated.h"

UCLASS()
class UE4_OPENCV_API AOpenCV_Reader : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AOpenCV_Reader(const FObjectInitializer& ObjectInitializer);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		USceneComponent* rootComp;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		UStaticMeshComponent* Screen_Raw;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		UStaticMeshComponent* Screen_Post;
	// The device ID opened by the Video Stream
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera, meta = (ClampMin = 0, UIMin = 0))
		int32 CameraID;
	// The device ID opened by the Video Stream
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera, meta = (ClampMin = 0, UIMin = 0))
		int32 VideoTrackID;
	// The rate at which the color data array and video texture is updated (in frames per second)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera, meta = (ClampMin = 0, UIMin = 0))
		float RefreshRate;
	// The refresh timer
	UPROPERTY(BlueprintReadWrite, Category = Camera)
		float RefreshTimer;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		float Brightness;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		float Multiply;
	// is the camera stream on
	UPROPERTY(BlueprintReadWrite, Category = Camera)
		bool isStreamOpen;
	// The videos width and height (width, height)
	UPROPERTY(BlueprintReadWrite, Category = Camera)
		FVector2D VideoSize;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		UMediaPlayer* Camera_MediaPlayer;
	// Camera Media Texture
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		UMediaTexture* Camera_MediaTexture;
	// Camera Render Target
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		UTextureRenderTarget2D* Camera_RenderTarget;
	// Camera Material Instance
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		UMaterialInstanceDynamic* Camera_MatRaw;
	// Camera Material Instance
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		UMaterialInstanceDynamic* Camera_MatPost;
	// Camera Texture
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Camera)
		UTexture2D* Camera_Texture2D;
	// Color Data
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Data)
		TArray<FColor> ColorData;// Blueprint Event called every time the video frame is updated
	UFUNCTION(BlueprintImplementableEvent, Category = Camera)
		void OnNextVideoFrame();
	// reads the current video frame
	UFUNCTION(BlueprintCallable, Category = Data)
		void ReadFrame();
	//OpenCV
	cv::Size cvSize;
	cv::Mat cvMat;
};
