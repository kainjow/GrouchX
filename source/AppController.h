//
//  AppController.h
//  GrouchX
//
//  Created by Kevin Wojniak on 2/27/11.
//  Copyright 2011 Kevin Wojniak. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AnimView.h"
#include "snd.h"

@interface AppController : NSWindowController <AnimViewDelegate>
{
	IBOutlet AnimView *view1;
	IBOutlet AnimView *view2;
	IBOutlet AnimView *view3;
	IBOutlet AnimView *view4;
	IBOutlet AnimView *view5;
	snd_t *sound1;
	snd_t *sound2;
	int whichSound;
}

- (IBAction)randomPlay:(id)sender;

@end
