//
//  AnimView.m
//  GrouchX
//
//  Created by Kevin Wojniak on 3/3/11.
//  Copyright 2011 Kevin Wojniak. All rights reserved.
//

#import "AnimView.h"
#include "rsrc.h"
#include "anim.h"


@implementation AnimView

@synthesize delegate;
@synthesize canPlay;

- (void)dealloc
{
	if (images) {
		CFRelease(images);
	}
	[super dealloc];
}

- (void)setImages:(CFArrayRef)imagesArray
{
	if (images != imagesArray) {
		if (images) {
			CFRelease(images);
		}
		images = (CFArrayRef)CFRetain(imagesArray);
	}
	[self setNeedsDisplay:YES];
}

- (void)play
{
	[self.delegate animViewWillPlay:self];
	// 0.055 = the sounds' duration (~2.8s) / number of images (51)
	timer = [[NSTimer scheduledTimerWithTimeInterval:0.055 target:self selector:@selector(animate) userInfo:nil repeats:YES] retain];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode];
}

- (void)animate
{
	imgFrame++;
	if (imgFrame == CFArrayGetCount(images)) {
		imgFrame = 0;
		[timer invalidate];
		[timer release];
		timer = nil;
		[self.delegate animViewDidFinishPlaying:self];
	}
	[self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect
{
	if (!images || !CFArrayGetCount(images)) {
		return;
	}
	
	CGRect bounds = NSRectToCGRect([self bounds]);
	CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
	
	CGImageRef img = (CGImageRef)CFArrayGetValueAtIndex(images, imgFrame);
	CGRect imgRect = CGRectMake(bounds.origin.x, bounds.origin.y, CGImageGetWidth(img), CGImageGetHeight(img));
	CGContextDrawImage(ctx, imgRect, img);
}

- (void)mouseDown:(NSEvent *)event
{
	if (!self.isPlaying && self.canPlay) {
		[self play];
	}
}

- (BOOL)isPlaying
{
	return (timer == nil ? NO : YES);
}

@end
