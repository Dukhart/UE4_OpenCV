// Copywrite Dukhart GNU v3.0

#pragma once

#include "CoreMinimal.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect.hpp"
#include "MediaTexture.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"
#include "OpenCV_FaceCapture.generated.h"

UCLASS()
class METAAVATAR_API AOpenCV_FaceCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpenCV_FaceCapture(const FObjectInitializer& ObjectInitializer);
	~AOpenCV_FaceCapture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The device ID opened by the Video Stream
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera, meta = (ClampMin = 0, UIMin = 0))
		int32 CameraID;
	// The device ID opened by the Video Stream
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera, meta = (ClampMin = 0, UIMin = 0))
		int32 VideoTrackID;
	// is the camera stream on
	UPROPERTY(BlueprintReadWrite, Category = Input)
		bool isStreamOpen;
	// is the camera stream on
	UPROPERTY(BlueprintReadWrite, Category = Input)
		bool shouldReadFrame;
	// The videos width and height (width, height)
	UPROPERTY(BlueprintReadWrite, Category = Input)
		FVector2D VideoSize;

	// Camera Media Player
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Input)
		UMediaPlayer* MediaPlayer;
	// Camera Media Texture
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Input)
		UMediaTexture* MediaTexture;

	// 8 bit Render Target (Pre Edit)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Materials)
		UTextureRenderTarget2D* RenderTarget_Raw;
	// 32 bit Render Target (Post Edit)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Materials)
		UTextureRenderTarget2D* RenderTarget_Post;

	// Material Camera raw Instance (Pre Edit)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Materials)
		UMaterialInstanceDynamic* Material_Raw;
	// Draws OpenCV_Texture2D (Post Edit)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Materials)
		UMaterialInstanceDynamic* Material_Post;

	// OpenCV Texture (Pre Edit)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = OpenCV)
		UTexture2D* OpenCV_Texture2D_Pre;
	// OpenCV Texture (Post Edit)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = OpenCV)
		UTexture2D* OpenCV_Texture2D_Post;

	// Color Data
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Data)
		TArray<FColor> ColorData;

	void LoadOpenCV();
	// reads the current video frame
	UFUNCTION(BlueprintCallable, Category = Data)
		bool ReadFrame();
	bool FindFace();
	// Camera controls
	UFUNCTION(BlueprintCallable, Category = Status)
		void NextCamera();
	UFUNCTION(BlueprintCallable, Category = Status)
		void NextVideoTrack();
	UFUNCTION(BlueprintImplementableEvent, Category = Camera)
		void ValidateVideoTrackID();
	UFUNCTION(BlueprintImplementableEvent, Category = Camera)
		void ValidateCameraID();

	//OpenCV
	cv::Size cvSize;
	cv::Mat cvMat;
	cv::Mat cvMat_gray;
	std::vector<cv::Rect> faces;

	cv::CascadeClassifier cvCascadeClassifier_Face;
	cv::CascadeClassifier cvCascadeClassifier_Eyes;

	FString Path_Cascade_Face = "ThirdParty/OpenCV/Cascades/haarcascades/haarcascade_frontalface_alt.xml";
	FString Path_Cascade_Eyes = "ThirdParty/OpenCV/Cascades/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
};
