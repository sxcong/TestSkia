// TestSkia.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <fstream>

#include "include/codec/SkCodec.h"
#include "include/core/SkImage.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPath.h"
#include "include/core/SkGraphics.h"

#include "include/core/SKFont.h"
#include "include/encode/SkPngEncoder.h" 
#include "include/core/SkStream.h" 

#include "include/core/SkFontMgr.h" 
#include "include/core/SkTypeface.h"
#include "include/core/SkFontStyle.h"

#include "include/ports/SkTypeface_win.h"
#include "skia/src/base/SkUTF.h"
#include "include/core/SkShader.h"

#ifdef _DEBUG
#pragma comment(lib, "lib/Debug/skia.dll.lib")
#else
#pragma comment(lib, "lib/Release/skia.dll.lib")
#endif


struct WatermarkConfig {
	std::string text = "test";
	SkColor color = SkColorSetARGB(128, 255, 0, 0);
	float fontSize = 32.0f;
	float xOffset = 320.0f;
	float yOffset = 320.0f;
	float rotateDeg = 0.0f;
};

sk_sp<SkFontMgr> fontMgr;
sk_sp<SkTypeface> typeface;

WatermarkConfig config;



sk_sp<SkImage> drawOverlayOnBackground(
	sk_sp<SkImage> backgroundImage,
	sk_sp<SkImage> overlayImage,
	float dstX, float dstY,
	float overlayWidth,   
	float overlayHeight)  
{
	if (!backgroundImage || !overlayImage) {
		return nullptr;
	}

	int bgWidth = backgroundImage->width();
	int bgHeight = backgroundImage->height();


	sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(bgWidth, bgHeight));
	if (!surface) return nullptr;

	SkCanvas* canvas = surface->getCanvas();


	canvas->drawImage(backgroundImage, 0, 0);


	SkRect dstRect = SkRect::MakeXYWH(dstX, dstY, overlayWidth, overlayHeight);


	SkRect srcRect = SkRect::MakeIWH(overlayImage->width(), overlayImage->height());


	SkPaint paint;
	paint.setBlendMode(SkBlendMode::kSrcOver); 

	SkFilterMode fm = SkFilterMode::kLinear;

	sk_sp<SkShader> fShader = backgroundImage->makeShader(SkSamplingOptions(fm));
	paint.setShader(fShader);

	//: SkFilterMode::kNearest;
	canvas->drawImageRect(overlayImage, srcRect, SkFilterMode::kLinear, &paint);



	SkPaint textPaint;
	textPaint.setColor(config.color);
	textPaint.setAntiAlias(true);
	textPaint.setBlendMode(SkBlendMode::kSrcOver); 
	textPaint.setStyle(SkPaint::kFill_Style);

	textPaint.setColor(SK_ColorRED); // brush
	textPaint.setAlpha(255); // 


	SkFontStyle boldStyle(
		SkFontStyle::kBold_Weight,    
		SkFontStyle::kNormal_Width,
		SkFontStyle::kUpright_Slant
	);

	typeface = fontMgr->matchFamilyStyle("Arial", boldStyle);
	if (!typeface) 
	{
		typeface = fontMgr->matchFamilyStyle(nullptr, boldStyle); // 系统默认粗体
	}

	float fontSize = 24.0f;
	//SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 24);
	SkFont font(typeface, fontSize);

	SkRect rect[2] = { { 10, 20, 90, 110 }, { 40, 130, 140, 180 } };

	canvas->drawString("text1", rect[0].fLeft, rect[0].fBottom, font, textPaint);
	canvas->drawString("text2", rect[1].fLeft, rect[1].fBottom, font, textPaint);

	
	return surface->makeImageSnapshot();
}

void SkiaLoadImage() {


	fontMgr = SkFontMgr_New_GDI();

	// 1. Upload.bmp is bg
	sk_sp<SkData> bg_data = SkData::MakeFromFileName("Upload.bmp");
	sk_sp<SkImage> bg = SkImages::DeferredFromEncodedData(bg_data);

	//splash.bmp is thumb
	sk_sp<SkData> thumb_data = SkData::MakeFromFileName("splash.bmp");
	sk_sp<SkImage> thumb = SkImages::DeferredFromEncodedData(thumb_data);


	sk_sp<SkImage> result = drawOverlayOnBackground(bg, thumb, 50, 60, 100, 100);


	// output png
	SkFILEWStream outputStream("output.png");
	if (!outputStream.isValid()) { return; }
	SkPngEncoder::Options pngOptions;
	pngOptions.fZLibLevel = 6;

	SkPixmap pixmap;
	if (!result->peekPixels(&pixmap)) {
		std::cerr << "Failed to get pixels for output\n";
		return;
	}


	bool encodeSuccess = SkPngEncoder::Encode(&outputStream, pixmap, pngOptions);
	outputStream.flush();
}


int main()
{
    std::cout << "Hello World!\n";
	SkiaLoadImage();
	return 0;
}

