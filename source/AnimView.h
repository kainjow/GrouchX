//
//  AnimView.h
//  GrouchX
//
//  Created by Kevin Wojniak on 3/3/11.
//  Copyright 2011 Kevin Wojniak. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol AnimViewDelegate;

@interface AnimView : NSView
{
	CFArrayRef images;
	NSTimer *timer;
	CFIndex imgFrame;
	BOOL canPlay;
	id<AnimViewDelegate> delegate;
}

- (void)setImages:(CFArrayRef)imagesArray;

- (void)play;
@property (readonly) BOOL isPlaying;

@property (assign) id<AnimViewDelegate> delegate;
@property BOOL canPlay;

@end


@protocol AnimViewDelegate
@required
- (void)animViewWillPlay:(AnimView *)sender;
- (void)animViewDidFinishPlaying:(AnimView *)sender;
@end
