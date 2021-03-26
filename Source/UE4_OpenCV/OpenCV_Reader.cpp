#include "OpenCV_Reader.h"

// Sets default values
AOpenCV_Reader::AOpenCV_Reader(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = true;
	// ensure the root component exists
	if (!RootComponent)
		RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::KeepRelative, false);
	Screen_Raw = CreateDefaultSubobject<UStaticMeshComponent>("Screen Raw");
	Screen_Raw->AttachToComponent(RootComponent, rules);
	Screen_Post = CreateDefaultSubobject<UStaticMeshComponent>("Screen Post");
	Screen_Post->AttachToComponent(RootComponent, rules);
	Brightness = 0;
	Multiply = 1;
	// Initialize OpenCV and webcam properties
	CameraID = 0;
	VideoTrackID = 0;
	isStreamOpen = false;
	VideoSize = FVector2D(1920, 1080);
	RefreshRate = 30.0f;
}

// Called when the game starts or when spawned
void AOpenCV_Reader::BeginPlay() {
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Some warning message"));
	isStreamOpen = true;
	// Prepare the color data array
	ColorData.AddDefaulted(VideoSize.X * VideoSize.Y);
	// setup openCV
	cvSize = cv::Size(VideoSize.X, VideoSize.Y);
	cvMat = cv::Mat(cvSize, CV_8UC4, ColorData.GetData());
	// create dynamic texture
	Camera_Texture2D = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y, PF_B8G8R8A8);
#if WITH_EDITORONLY_DATA
	Camera_Texture2D->MipGenSettings = TMGS_NoMipmaps;
#endif
	Camera_Texture2D->SRGB = Camera_RenderTarget->SRGB;
}

void AOpenCV_Reader::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	RefreshTimer += DeltaTime;
	if (isStreamOpen && RefreshTimer >= 1.0f / RefreshRate) {
		RefreshTimer -= 1.0f / RefreshRate;
		ReadFrame();
		OnNextVideoFrame();
	}
}

void AOpenCV_Reader::ReadFrame() {
	if (!Camera_Texture2D || !Camera_RenderTarget) return;
	// Read the pixels from the RenderTarget and store them in a FColor array
	FRenderTarget* RenderTarget = Camera_RenderTarget->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(ColorData);
	// Get the color data
	cvMat = cv::Mat(cvSize, CV_8UC4, ColorData.GetData());
	cvMat.convertTo(cvMat, -1, Multiply, Brightness);
	// show the openCV window
	if (!cvMat.empty())
		cv::imshow("Display", cvMat);
	// Lock the texture so we can read / write to it
	void* TextureData = Camera_Texture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	const int32 TextureDataSize = ColorData.Num() * 4;
	// set the texture data
	FMemory::Memcpy(TextureData, ColorData.GetData(), TextureDataSize);
	// Unlock the texture
	Camera_Texture2D->PlatformData->Mips[0].BulkData.Unlock();
	// Apply Texture changes to GPU memory
	Camera_Texture2D->UpdateResource();
}
