// Copywrite Dukhart GNU v3.0
#include "OpenCVReader.h"

AOpenCVReader::AOpenCVReader(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    // ensure the root component exists
    if (!RootComponent)
        RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
    FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::KeepRelative, false);

    // Create and attach sub components
    Screen_Raw = CreateDefaultSubobject<UStaticMeshComponent>("Screen Raw");
    Screen_Raw->AttachToComponent(RootComponent, rules);
    Screen_Post = CreateDefaultSubobject<UStaticMeshComponent>("Screen Post");
    Screen_Post->AttachToComponent(RootComponent, rules);
    Scene_Capture = CreateDefaultSubobject<USceneCaptureComponent2D>("Scene Capture");
    Scene_Capture->AttachToComponent(RootComponent, rules);

    // setup property defaults
    InputMode = EInputMode::Camera;
    ColorMode = ETextureRenderTargetFormat::RTF_RGBA8;

    Brightness = 0;
    Multiply = 1;
    // Initialize OpenCV and webcam properties
    CameraID = 0;
    VideoTrackID = 0;
    isStreamOpen = false;
    applyToPlayerScreen = true;
    VideoSize = FVector2D(1920, 1080);
    RefreshRate = 30.0f;
}
// Called when the game starts or when spawned
void AOpenCVReader::BeginPlay()
{
    Super::BeginPlay();
    isStreamOpen = true;
    // Prepare the color data array
    ColorData.AddDefaulted(VideoSize.X * VideoSize.Y);
    // setup openCV
    cvSize = cv::Size(VideoSize.X, VideoSize.Y);

    int cvColorMode = GetColorMode_CV();
    cvMat = cv::Mat(cvSize, cvColorMode, ColorData.GetData());
    // create dynamic texture
    OpenCV_Texture2D = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y, PF_B8G8R8A8);

#if WITH_EDITORONLY_DATA
    OpenCV_Texture2D->MipGenSettings = TMGS_NoMipmaps;
#endif
    OpenCV_Texture2D->SRGB = RenderTarget->SRGB;
}
void AOpenCVReader::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    RefreshTimer += DeltaTime;
    if (isStreamOpen && RefreshTimer >= 1.0f / RefreshRate)
    {
        RefreshTimer -= 1.0f / RefreshRate;
        ReadFrame();
        OnNextVideoFrame();
    }
}
int AOpenCVReader::GetColorMode_CV() {
    int cvColorMode = CV_8UC4;
    switch (ColorMode)
    {
    case ETextureRenderTargetFormat::RTF_RGBA8:
        cvColorMode = CV_8UC4;
        break;
    case ETextureRenderTargetFormat::RTF_RGBA16f:
        UE_LOG(LogTemp, Warning, TEXT("16 bit not yet supported, Only 8 bit colour implemented currently will always return CV_8UC4"));
        cvColorMode = CV_16FC4;
        break;
    case ETextureRenderTargetFormat::RTF_RGBA32f:
        UE_LOG(LogTemp, Warning, TEXT("32 bit not yet supported, Only 8 bit colour implemented currently will always return CV_8UC4"));
        cvColorMode = CV_32FC4;
        break;
    default:
        // TODO Error unrecognized color format!
        cvColorMode = CV_8UC4;
        break;
    }
    cvColorMode = CV_8UC4;
    return cvColorMode;
}
bool AOpenCVReader::ReadFrame() {
    if (!OpenCV_Texture2D || (!RenderTarget && !RenderTarget_32Bit)) return false;
    // Read the pixels from the RenderTarget and store them in a FColor array
    //TArray<FColor> SurfData;
    FRenderTarget* renderTarget;
    if (InputMode == EInputMode::Player)
        renderTarget = RenderTarget_32Bit->GameThread_GetRenderTargetResource();
    else
        renderTarget = RenderTarget->GameThread_GetRenderTargetResource();

    renderTarget->ReadPixels(ColorData);
    int cvColorMode = GetColorMode_CV();
    // Get the color data
    cvMat = cv::Mat(cvSize, cvColorMode, ColorData.GetData());
    // do fun stuff here
    cvMat.convertTo(cvMat, -1, Multiply, Brightness);

    // show the openCV window
    if (!cvMat.empty())
        cv::imshow("Display", cvMat);

    //Wrapped in a render command for performance
    ENQUEUE_RENDER_COMMAND(WriteOpenCVTexture)(
        [RTarget = RenderTarget, RTexture = OpenCV_Texture2D, ColorD = ColorData](FRHICommandList& RHICmdList)
        {
            void* TextureData = RTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            const int32 TextureDataSize = ColorD.Num() * 4;
            // set the texture data
            FMemory::Memcpy(TextureData, ColorD.GetData(), TextureDataSize);
            RTexture->PlatformData->Mips[0].BulkData.Unlock();
            // Apply Texture changes to GPU memory
            RTexture->UpdateResource();
        });
    return true;
}
// Warning Pixel format mismatch
//EPixelFormat::PF_A2B10G10R10; // back buffer
//EPixelFormat::PF_B8G8R8A8; // target
void AOpenCVReader::OnBackBufferReady(SWindow& SlateWindow, const FTexture2DRHIRef& BackBuffer) {
    ensure(IsInRenderingThread());
    FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
    FRHITexture2D* CachedTexture = RenderTarget_32Bit->Resource->TextureRHI->GetTexture2D();

    FRHICopyTextureInfo CopyInfo;

    RHICmdList.CopyTexture(BackBuffer, CachedTexture, CopyInfo);

    if (applyToPlayerScreen && RenderTarget_PlayerScreen) {
        CachedTexture = RenderTarget_PlayerScreen->Resource->TextureRHI->GetTexture2D();
        RHICmdList.CopyTexture(CachedTexture, BackBuffer, CopyInfo);
    }
}

//increment camera by one then validate exists
void AOpenCVReader::NextCamera()
{
    CameraID += 1;
    ValidateCameraID();
}
//increment video track by one then validate exists
void AOpenCVReader::NextVideoTrack()
{
    VideoTrackID += 1;
    ValidateVideoTrackID();
}

// bind our delegate to enable player mode
void AOpenCVReader::EnablePlayerMode() {
    InputMode = EInputMode::Player;
    OnBackBufferReadyToPresent = FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().AddUObject(this, &AOpenCVReader::OnBackBufferReady);
}
// unbind delegate to disable player mode
void AOpenCVReader::DisablePlayerMode() {
    OnBackBufferReadyToPresent.Reset();
}

void AOpenCVReader::EnableCameraMode() {
    InputMode = EInputMode::Camera;
}

void AOpenCVReader::DisableCameraMode() {

}

void AOpenCVReader::EnableSceneMode() {
    InputMode = EInputMode::Scene;
}

void AOpenCVReader::DisableSceneMode() {

}