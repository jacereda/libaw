/*
  Copyright (c) 2008-2010, Jorge Acereda Maciá
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:
        
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
        
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the author nor the names of its contributors
  may be used to endorse or promote products derived from this
  software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "aw.h"
#include "awos.h"
#include "sysgl.h"
#include "co.h"
#include <OpenGLES/ES1/glext.h>
#include <assert.h>

#include <Foundation/Foundation.h>
#include <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

struct _aw {
        awHeader hdr;
};

struct _ac {
        acHeader hdr;
};

@interface glview : UIView {}
@end
@interface awdelegate : NSObject <UIApplicationDelegate, UITextFieldDelegate> {
        UIWindow * win;
        glview* view; 
        EAGLContext * ctx;
        CADisplayLink * dpylink;
	UITextField * tf;
@public
	co * comain;
	co * coaw;
	int awdone;
        aw w;
        ac c;
}
@end

awdelegate * getDelegate() {
	return (awdelegate*)coData(coCurrent());
}

static void dgot(int e, int x, int y) {
        got(&getDelegate()->w, e, x, y);
}

static void fakekeyuni(unsigned key, unsigned unicode) {
	dgot(AW_EVENT_DOWN, key, 0);
	dgot(AW_EVENT_UNICODE, unicode, 0);
	dgot(AW_EVENT_UP, key, 0);
}

static unsigned mapkey(unsigned k) {
	unsigned ret;
	switch (k) {
	case 'A': ret = AW_KEY_A; break;
	case 'B': ret = AW_KEY_B; break;
	case 'C': ret = AW_KEY_C; break;
	case 'D': ret = AW_KEY_D; break;
	case 'E': ret = AW_KEY_E; break;
	case 'F': ret = AW_KEY_F; break;
	case 'G': ret = AW_KEY_G; break;
	case 'H': ret = AW_KEY_H; break;
	case 'I': ret = AW_KEY_I; break;
	case 'J': ret = AW_KEY_J; break;
	case 'K': ret = AW_KEY_K; break;
	case 'L': ret = AW_KEY_L; break;
	case 'M': ret = AW_KEY_M; break;
	case 'N': ret = AW_KEY_N; break;
	case 'O': ret = AW_KEY_O; break;
	case 'P': ret = AW_KEY_P; break;
	case 'Q': ret = AW_KEY_Q; break;
	case 'R': ret = AW_KEY_R; break;
	case 'S': ret = AW_KEY_S; break;
	case 'T': ret = AW_KEY_T; break;
	case 'U': ret = AW_KEY_U; break;
	case 'V': ret = AW_KEY_V; break;
	case 'W': ret = AW_KEY_W; break;
	case 'X': ret = AW_KEY_X; break;
	case 'Y': ret = AW_KEY_Y; break;
	case 'Z': ret = AW_KEY_Z; break;
	case '0': ret = AW_KEY_0; break;
	case '1': ret = AW_KEY_1; break;
	case '2': ret = AW_KEY_2; break;
	case '3': ret = AW_KEY_3; break;
	case '4': ret = AW_KEY_4; break;
	case '5': ret = AW_KEY_5; break;
	case '6': ret = AW_KEY_6; break;
	case '7': ret = AW_KEY_7; break;
	case '8': ret = AW_KEY_8; break;
	case '9': ret = AW_KEY_9; break;
	case '=': ret = AW_KEY_EQUAL; break;
	case '\b': ret = AW_KEY_DELETE; break;
	case '\n': ret = AW_KEY_RETURN; break;
	default: ret = AW_KEY_NONE;
	}
	return ret;
}

static void fakekey(unsigned unicode) {
	int upper = isupper(unicode);
	int luni = toupper(unicode);
	if (upper)
		dgot(AW_EVENT_DOWN, AW_KEY_SHIFT, 0);
	fakekeyuni(mapkey(luni), unicode);
	if (upper)
		dgot(AW_EVENT_UP, AW_KEY_SHIFT, 0);
}

@implementation glview
+ (Class) layerClass {
        return [CAEAGLLayer class];
}

- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) e
{
        UITouch* touch = [touches anyObject];
        CGPoint loc = [touch locationInView: self];
        dgot(AW_EVENT_MOTION, loc.x, loc.y);
        dgot(AW_EVENT_DOWN, AW_KEY_MOUSELEFT, 0);
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) e
{
        UITouch* touch = [touches anyObject];
        CGPoint loc = [touch locationInView: self];
        dgot(AW_EVENT_MOTION, loc.x, loc.y);
        dgot(AW_EVENT_UP, AW_KEY_MOUSELEFT, 0);
}


- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) e
{
        UITouch* touch = [touches anyObject];
        CGPoint loc = [touch previousLocationInView: self];
        dgot(AW_EVENT_MOTION, loc.x, loc.y);
}

@end

void awentry(void * data) {
	extern int fakemain(int argc, char ** argv);
	int argc = 1;
	char * argv0 = "awiphone";
	fakemain(argc, &argv0);
	coSwitchTo(getDelegate()->comain);
	assert(0);
}

@implementation awdelegate
- (BOOL) textField: (UITextField *)tf 
shouldChangeCharactersInRange: (NSRange)range 
 replacementString: (NSString *)s {
        int sl = [s length];
        int i;
	if (!sl) 
		fakekey('\b');
        for (i = 0; i < sl; i++)
		fakekey([s characterAtIndex: i]);
	return NO;
}

- (BOOL)textFieldShouldReturn: (UITextField*)tf {
	fakekey('\n');
	return NO;
}

- (void) update {
        coSwitchTo(coaw);
        [ctx presentRenderbuffer: GL_RENDERBUFFER_OES];
}

- (void) applicationDidFinishLaunching: (UIApplication*) application 
{
        comain = coMain(self);
        coaw = coNew(awentry, self);
        CGRect r = [[UIScreen mainScreen] bounds];
        win = [[UIWindow alloc] initWithFrame: r];
        view = [[glview alloc] initWithFrame: r];
	tf = [[UITextField alloc] initWithFrame: r];
	[tf becomeFirstResponder];
	[tf setDelegate: self];
	[tf setAutocapitalizationType: UITextAutocapitalizationTypeNone];
	[tf setAutocorrectionType: UITextAutocorrectionTypeNo];
	[tf setEnablesReturnKeyAutomatically: NO];
	[tf setText: @" "];
	[tf setHidden: YES];
        [win makeKeyAndVisible]; 
        [win addSubview: view]; 
	[win addSubview: tf];
        ctx = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES2];
        [EAGLContext setCurrentContext: ctx];
        GLuint cb;
        GLuint fb;
        glGenFramebuffersOES(1, &fb);
        glGenRenderbuffersOES(1, &cb);
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES , cb);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, 
                                     GL_COLOR_ATTACHMENT0_OES, 
                                     GL_RENDERBUFFER_OES, 
                                     cb);
        [ctx renderbufferStorage: GL_RENDERBUFFER_OES fromDrawable:
                     (CAEAGLLayer *) [view layer]];
        dpylink = [CADisplayLink displayLinkWithTarget: self
                                 selector: @selector(update)];
        [dpylink setFrameInterval: 1];
        [dpylink addToRunLoop: [NSRunLoop currentRunLoop] 
                 forMode: NSDefaultRunLoopMode];
        dgot(AW_EVENT_RESIZE, r.size.width, r.size.height);
}

- (void) dealloc
{
        [dpylink release];
        [win release];
        [view release];
        [ctx release];
        [super dealloc];
}
@end

int main(int argc, char *argv[]) {
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        int ret = UIApplicationMain(argc, argv, nil, @"awdelegate");
        [pool release];
        return ret;
}

int awosInit() {
        return 1;
}

int awosEnd() {
        return 1;
}

int awosSetTitle(aw * w, const char * t) {
        return 1;
}

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
        return &getDelegate()->w;
}

int awosClose(aw * w) {
        return 1;
}

int awosSwapBuffers(aw * w) {
        coSwitchTo(getDelegate()->comain);
        return 1;
}

int awosShow(aw * w) {
        return 1;
}

int awosHide(aw * w) {
        return 1;
}

void awosPollEvent(aw * w) {
}

int awosSetSwapInterval(aw * w, int i) {
        return 1;
}

int awosClearCurrent(aw * w) {
        return 1;
}

int awosMakeCurrent(aw * w, ac * c) {
        return 1;
}

ac * acosNew(ac * share) {
        return &getDelegate()->c;
}

int acosDel(ac * cc) {
        return 1;
}

