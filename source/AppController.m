//
//  AppController.m
//  GrouchX
//
//  Created by Kevin Wojniak on 2/27/11.
//  Copyright 2011 Kevin Wojniak. All rights reserved.
//

#import "AppController.h"
#include "rsrc.h"
#include "anim.h"


@implementation AppController

- (void)setCanPlayAll:(BOOL)flag
{
	view1.canPlay = flag;
	view2.canPlay = flag;
	view3.canPlay = flag;
	view4.canPlay = flag;
	view5.canPlay = flag;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notif
{
	NSString *path = [[NSBundle mainBundle] pathForResource:@"grouch.rsrc" ofType:nil];
	
	rsrc_t *rsrc = rsrc_open([path fileSystemRepresentation], rsrc_fork_data);
	if (rsrc) {
		uint32_t size;
		char *data = rsrc_get_resource(rsrc, 'ANIM', 301, &size);
		if (data) {
			CFArrayRef images = anim_create_images(data, size);
			[view1 setImages:images];
			[view2 setImages:images];
			[view3 setImages:images];
			[view4 setImages:images];
			[view5 setImages:images];
			if (images) {
				CFRelease(images);
			}
			view1.delegate = self;
			view2.delegate = self;
			view3.delegate = self;
			view4.delegate = self;
			view5.delegate = self;
			free(data);
		}
		
		data = rsrc_get_resource(rsrc, 'snd ', 300, &size);
		if (data) {
			sound1 = snd_new(data, size);
			free(data);
		}

		data = rsrc_get_resource(rsrc, 'snd ', 301, &size);
		if (data) {
			sound2 = snd_new(data, size);
			free(data);
		}
		
		rsrc_close(rsrc);
	}
	
	[self setCanPlayAll:YES];
	[self showWindow:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return YES;
}

- (IBAction)randomPlay:(id)sender
{
	NSArray *views = [NSArray arrayWithObjects:view1, view2, view3, view4, view5, nil];
	for (AnimView *view in views) {
		if (view.isPlaying) {
			return;
		}
	}
	[self setCanPlayAll:NO];
	[(AnimView *)[views objectAtIndex:arc4random() % [views count]] play];
}

- (void)animViewWillPlay:(AnimView *)sender
{
	[self setCanPlayAll:NO];
	if (whichSound == 0) {
		snd_play(sound1);
	} else {
		snd_play(sound2);
	}
	whichSound = !whichSound;
}

- (void)animViewDidFinishPlaying:(AnimView *)sender
{
	[self setCanPlayAll:YES];
}

@end
