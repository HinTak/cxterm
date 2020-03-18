! BEGIN_CXTERM_RESOURCE
!
XTerm*vtMenu*cxtermconfig*Label:	Hanzi Input Configuration

! CXterm ANSI color configuration
cxterm*fcolor0:     #000000
cxterm*fcolor1:     #8b0000
cxterm*fcolor2:     #006400
cxterm*fcolor3:     #cdcd00
cxterm*fcolor4:     #00008b
cxterm*fcolor5:     #cd00cd
cxterm*fcolor6:     #008b8b
cxterm*fcolor7:     #d3d3d3
cxterm*fcolorh0:    #a9a9a9
cxterm*fcolorh1:    #ff0000
cxterm*fcolorh2:    #00ff00
cxterm*fcolorh3:    #ffff00
cxterm*fcolorh4:    #0000ff
cxterm*fcolorh5:    #ff00ff
cxterm*fcolorh6:    #00ffff
cxterm*fcolorh7:    #ffffff

cxterm*bcolor0:     #000000
cxterm*bcolor1:     #8b0000
cxterm*bcolor2:     #006400
cxterm*bcolor3:     #cdcd00
cxterm*bcolor4:     #00008b
cxterm*bcolor5:     #cd00cd
cxterm*bcolor6:     #008b8b
cxterm*bcolor7:     #d3d3d3

cxterm*configPopup*foreground: #000000
cxterm*configPopup*background: #d3d3d3

cxterm*hilight:    #ffffff
cxterm*foreground: #d3d3d3
cxterm*background: #000000

cxterm*bbs: on
cxterm*usebold: off

!
! resource for GB cxterm (Simplified Chinese as used in Mainland China)
!
cxterm*HanziEncoding:		GB
cxterm*hanziInputDir:	cxterm-5.1/dict/gb
cxterm*hanziAssociation:	simple.lx
cxterm*hanziFont:		hanzigb16fs
cxterm*hanziBoldFont:		hanzigb16st
cxterm*font:			8x16
cxterm*boldFont:
cxterm*fontMenu.Label:		VT Fonts
cxterm*fontMenu*fontdefault*Label:	Default
cxterm*fontMenu*font1*Label:	Normal Ascii
cxterm*VT100*font1:		8x16
cxterm*fontMenu*font2*Label:	Large Ascii
cxterm*VT100*font2:		12x24
cxterm*fontMenu*font3*Label:	Bold Hanzi
cxterm*VT100*font3:		hanzigb16st
cxterm*fontMenu*font4*Label:	Thin Hanzi
cxterm*VT100*font4:		hanzigb16fs
cxterm*fontMenu*font5*Label:	Large Hanzi
cxterm*VT100*font5:		hanzigb24st
cxterm*fontMenu*font6*Label:	Normal Hanzi (GR)
cxterm*VT100*font6:		cclib16st
cxterm*fontMenu*fontescape*Label:	Escape Sequence
cxterm*fontMenu*fontsel*Label:		Selection
cxterm*VT100.Translations: #override\
 ~Shift ~Meta <KeyPress> Control_R: set-HZ-parameter(input-conv=wait_toggle)	\n\
 ~Shift ~Meta <KeyRelease> Control_R: set-HZ-parameter(input-conv=toggle)	\n\
 ~Shift ~Meta <KeyPress> Control_L: set-HZ-parameter(input-conv=wait_toggle)	\n\
 ~Shift ~Meta <KeyRelease> Control_L: set-HZ-parameter(input-conv=toggle)	\n\
 ~Shift <KeyPress> F1:	set-HZ-parameter(input-conv=wait_toggle)	\n\
 ~Shift <KeyRelease> F1:	set-HZ-parameter(input-conv=toggle)	\n\
  Shift <KeyPress> F1:	switch-HZ-mode(ASCII)		\n\
 ~Shift <KeyPress> F2:	switch-HZ-mode(IC)		\n\
  Shift <KeyPress> F2:	switch-HZ-mode(QW)		\n\
	<KeyPress> F3:	popup-panel(config)		\n\
 ~Shift <KeyPress> F4:  switch-HZ-mode(TONEPY)          \n\
  Shift <KeyPress> F4:  switch-HZ-mode(SW)              \n\
 ~Shift <KeyPress> F5:	switch-HZ-mode(QJ)		\n\
  Shift <KeyPress> F5:	switch-HZ-mode(Punct)		\n\
 ~Shift <KeyPress> F6:	switch-HZ-mode(PY)		\n\
  Shift <KeyPress> F6:	switch-HZ-mode(CCDOSPY)		\n\
 ~Shift <KeyPress> F7:	switch-HZ-mode(WuBi)		\n\
  Shift <KeyPress> F7:	switch-HZ-mode(CangJie)		\n\
 ~Shift <KeyPress> F8:	switch-HZ-mode(English)		\n\
  Shift <KeyPress> F8:	switch-HZ-mode(CTLau)		\n\
 ~Shift <KeyPress> F9:	switch-HZ-mode(UCDOSPY)		\n\
  Shift <KeyPress> F9:	switch-HZ-mode(QianMa)		\n\
 ~Shift	<KeyPress> F10:	switch-HZ-mode(ASCII)		\n\
	<KeyPress> F10:	switch-HZ-mode(TeleCode)	\n\
 ~Shift <KeyPress> F11:	switch-HZ-mode(HIRAGANA)	\n\
  Shift <KeyPress> F11:	switch-HZ-mode(KATAKANA)	\n\
  ~Meta ~Ctrl <Btn1Down>:	select-start() click-HZ-area()	\n\
  ~Meta <KeyPress> Escape:	insert() switch-HZ-mode(ASCII)

!
! resource for BIG5 cxterm (Traditional Chinese as used in Taiwan/Hong Kong)
!
cxtermb5*HanziEncoding:		BIG5
cxtermb5*hanziInputDir:	cxterm-5.1/dict/big5
cxtermb5*hanziAssociation:	simple.lx
cxtermb5*hanziFont:		hku16et
cxtermb5*hanziBoldFont:
cxtermb5*font:			8x16
cxtermb5*boldFont:
cxtermb5*fontMenu.Label:	VT Fonts
cxtermb5*fontMenu*fontdefault*Label:	Default
cxtermb5*fontMenu*font1*Label:	Normal Ascii
cxtermb5*VT100*font1:		8x16
cxtermb5*fontMenu*font2*Label:	Medium Ascii
cxtermb5*VT100*font2:		10x20
cxtermb5*fontMenu*font3*Label:	Large Ascii
cxtermb5*VT100*font3:		12x24
cxtermb5*fontMenu*font4*Label:	Normal Hanzi
cxtermb5*VT100*font4:		taipei16
cxtermb5*fontMenu*font5*Label:	Medium Hanzi
cxtermb5*VT100*font5:		taipei20
cxtermb5*fontMenu*font6*Label:	Large Hanzi
cxtermb5*VT100*font6:		taipei24
cxtermb5*fontMenu*fontescape*Label:	Escape Sequence
cxtermb5*fontMenu*fontsel*Label:		Selection
cxtermb5*VT100.Translations: #override\
 ~Shift ~Meta <KeyPress> Control_R: set-HZ-parameter(input-conv=wait_toggle)	\n\
 ~Shift ~Meta <KeyRelease> Control_R: set-HZ-parameter(input-conv=toggle)	\n\
 ~Shift ~Meta <KeyPress> Control_L: set-HZ-parameter(input-conv=wait_toggle)	\n\
 ~Shift ~Meta <KeyRelease> Control_L: set-HZ-parameter(input-conv=toggle)	\n\
 ~Shift <KeyPress> F1:	set-HZ-parameter(input-conv=wait_toggle)	\n\
 ~Shift <KeyRelease> F1:	set-HZ-parameter(input-conv=toggle)	\n\
  Shift <KeyPress> F1:	switch-HZ-mode(ASCII)	\n\
	<KeyPress> F2:	switch-HZ-mode(IC)	\n\
	<KeyPress> F3:	popup-panel(config)	\n\
	<KeyPress> F4:	switch-HZ-mode(PY)	\n\
 ~Shift <KeyPress> F5:	switch-HZ-mode(QJ)	\n\
  Shift <KeyPress> F5:	switch-HZ-mode(Punct)	\n\
 ~Shift <KeyPress> F6:	switch-HZ-mode(ETZY)	\n\
  Shift <KeyPress> F6:	switch-HZ-mode(ZOZY)	\n\
 ~Shift <KeyPress> F7:	switch-HZ-mode(Simplex)	\n\
  Shift <KeyPress> F7:	switch-HZ-mode(CangJie)	\n\
	<KeyPress> F8:	switch-HZ-mode(English)	\n\
 ~Shift <KeyPress> F9:	switch-HZ-mode(4Corner)	\n\
  Shift <KeyPress> F9:	switch-HZ-mode(HSU)	\n\
 ~Shift <KeyPress> F10:	switch-HZ-mode(ARRAY30)	\n\
  Shift <KeyPress> F10:	switch-HZ-mode(Boshiamy)\n\
  ~Meta <Btn1Down>:	select-start() click-HZ-area()	\n\
  ~Meta <KeyPress> Escape:	insert() switch-HZ-mode(ASCII)

!
! resource for JIS (Japanese) cxterm
!
cxtermjis*HanziEncoding:	JIS
cxtermjis*hanziInputDir:	cxterm-5.1/dict/jis
cxtermjis*hanziFont:		jiskan16
cxtermjis*hanziBoldFont:
cxtermjis*font:			8x16
cxtermjis*boldFont:
cxtermjis*fontMenu.Label:  	VT Fonts
cxtermjis*fontMenu*fontdefault*Label:	Default
cxtermjis*fontMenu*font1*Label:	Normal Ascii
cxtermjis*VT100*font1:		8x16
cxtermjis*fontMenu*font2*Label:	Smaller Ascii
cxtermjis*VT100*font2:		7x14
cxtermjis*fontMenu*font3*Label:	Large Ascii
cxtermjis*VT100*font3:		12x24
cxtermjis*fontMenu*font4*Label:	Normal Kanji
cxtermjis*VT100*font4:		jiskan16
cxtermjis*fontMenu*font5*Label:	Smaller Kanji
cxtermjis*VT100*font5:		k14
cxtermjis*fontMenu*font6*Label:	Large Kanji
cxtermjis*VT100*font6:		jiskan24
cxtermjis*fontMenu*fontescape*Label:	Escape Sequence
cxtermjis*fontMenu*fontsel*Label:		Selection
cxtermjis*VT100.Translations: #override\
	<KeyPress> F1:	switch-HZ-mode(ASCII)	\n\
	<KeyPress> F2:	switch-HZ-mode(IC)	\n\
	<KeyPress> F3:	popup-panel(config)	\n\
	<KeyPress> F4:	switch-HZ-mode(ROMKANA)	\n\
	<KeyPress> F5:	switch-HZ-mode(TCODE)	\n\
	<KeyPress> F6:	switch-HZ-mode(QJ)	\n\
	<KeyPress> F7:	switch-HZ-mode(Punct)	\n\
  ~Meta <Btn1Down>:	select-start() click-HZ-area()	\n\
  ~Meta <KeyPress> Escape:	insert() switch-HZ-mode(ASCII)

!
! resource for KSC (Korean) cxterm
!
cxtermks*HanziEncoding:		KS
cxtermks*hanziInputDir:	cxterm-5.1/dict/ks
cxtermks*hanziAssociation:	simple.lx
cxtermks*hanziFont:		hanglm16
cxtermks*hanziBoldFont:
cxtermks*font:			8x16
cxtermks*boldFont:
cxtermks*fontMenu.Label:  	VT Fonts
cxtermks*fontMenu*fontdefault*Label:	Default
cxtermks*fontMenu*font1*Label:	Normal Ascii
cxtermks*VT100*font1:		8x16
cxtermks*fontMenu*font2*Label:	Large Ascii
cxtermks*VT100*font2:		12x24
cxtermks*fontMenu*font3*Label:	Normal Hangul
cxtermks*VT100*font3:		hanglm16
cxtermks*fontMenu*font4*Label:	Gothic Hangul
cxtermks*VT100*font4:		hanglg16
cxtermks*fontMenu*font5*Label:	Large Hangul
cxtermks*VT100*font5:		hanglm24
cxtermks*fontMenu*font6*Label:	
cxtermks*VT100*font6:	
cxtermks*fontMenu*fontescape*Label:	Escape Sequence
cxtermks*fontMenu*fontsel*Label:		Selection
cxtermks*VT100.Translations: #override\
	<KeyPress> F1:	switch-HZ-mode(ASCII)	\n\
	<KeyPress> F2:	switch-HZ-mode(IC)	\n\
	<KeyPress> F3:	popup-panel(config)	\n\
	<KeyPress> F4:	switch-HZ-mode(HANGUL)	\n\
	<KeyPress> F5:	switch-HZ-mode(HANJA)	\n\
	<KeyPress> F6:	switch-HZ-mode(QJ)	\n\
	<KeyPress> F7:	switch-HZ-mode(Punct)	\n\
  ~Meta <Btn1Down>:	select-start() click-HZ-area()	\n\
  ~Meta <KeyPress> Escape:	insert() switch-HZ-mode(ASCII)

 XTerm*configPopup*Command.shapeStyle:	oval
 XTerm*configPopup*Command.internalWidth:	10
 XTerm*configPopup*Paned*showGrip:	false


!{ (paned)  frame
 XTerm*configPopup.frame*left:		chainleft
 XTerm*configPopup.frame*right:		chainleft

!  { (label)  frame.title
   XTerm*configPopup.frame.title.label:		CXTERM input configuration
!  } (label)

!  { (form)  frame.id

!    { (label)  frame.id.hzinputdir
     XTerm*configPopup.frame.id.hzinputdir.label:		INPUT SEARCH PATH
     XTerm*configPopup.frame.id.hzinputdir.borderWidth:	0
!    } (label)

!    { (asciiText)  frame.id.value
     XTerm*configPopup.frame.id.value.scrollHorizontal:	whenneeded
     XTerm*configPopup.frame.id.value.numCols:		30
     XTerm*configPopup.frame.id.value.translations:	#override\
	<Key>Return:	no-op(RingBell)	\n\
	Ctrl<Key>J:	no-op(RingBell) \n\
	Ctrl<Key>M:	no-op(RingBell)
!    } (asciiText)

!    { (command)  frame.id.apply
     XTerm*configPopup.frame.id.apply.label:		apply
!    } (command)
!  } (form)

!  { (form)  frame.im

!    { (label)  frame.im.input-method
     XTerm*configPopup.frame.im.input-method.label:		CURRENT INPUT METHOD
     XTerm*configPopup.frame.im.input-method.borderWidth:	0
!    } (label)

!    { (asciiText)  frame.im.value
     XTerm*configPopup.frame.im.value.scrollHorizontal:	whenneeded
     XTerm*configPopup.frame.im.value.numCols:		30
     XTerm*configPopup.frame.im.value.translations:	#override\
	<Key>Return:	no-op(RingBell)	\n\
	Ctrl<Key>J:	no-op(RingBell) \n\
	Ctrl<Key>M:	no-op(RingBell)
!    } (asciiText)

!    { (command)  frame.im.apply
     XTerm*configPopup.frame.im.apply.label:		apply
!    } (command)

!    { (label)  frame.im.choose-from
     XTerm*configPopup.frame.im.choose-from.borderWidth:	0
     XTerm*configPopup.frame.im.choose-from.justify:		right
     XTerm*configPopup.frame.im.choose-from.label:		or choose from:
!    } (label)

!    { (viewport)  frame.im.viewport
     XTerm*configPopup.frame.im.viewport.allowVert:	true
     XTerm*configPopup.frame.im.viewport.useRight:		true

!      { (list)  frame.im.viewport.list
       XTerm*configPopup.frame.im.viewport.list.Columns:	true
       XTerm*configPopup.frame.im.viewport.list.numRows:	8
       XTerm*configPopup.frame.im.viewport.list.numCols:	30
!      } (list)
!    } (viewport)
!  } (form)

!  { (form)  frame.ip
   XTerm*configPopup.frame.ip.Label.borderWidth:	0
   XTerm*configPopup.frame.ip.MenuButton.justify:	left

!    { (label)  frame.ip.parameter
     XTerm*configPopup.frame.ip.parameter.label:		INPUT PARAMETERS
!    } (label)

!    { (label)  frame.ip.auto-select
     XTerm*configPopup.frame.ip.auto-select.justify:		left
     XTerm*configPopup.frame.ip.auto-select.label:		Auto Selection
!    } (label)

!    { (label)  frame.ip.auto-segment
     XTerm*configPopup.frame.ip.auto-segment.justify:		left
     XTerm*configPopup.frame.ip.auto-segment.label:		Auto Segmentation
!    } (label)

!    { (label)  frame.ip.assocation
     XTerm*configPopup.frame.ip.assocation.justify:		left
     XTerm*configPopup.frame.ip.assocation.label:		Association
!    } (label)

!    { (label)  frame.ip.eq1
     XTerm*configPopup.frame.ip.eq1.label:		=
!    } (label)

!    { (label)  frame.ip.eq2
     XTerm*configPopup.frame.ip.eq2.label:		=
!    } (label)

!    { (label)  frame.ip.eq3
     XTerm*configPopup.frame.ip.eq3.label:		=
!    } (label)

!    { (menuButton)  frame.ip.sl-default
     XTerm*configPopup.frame.ip.sl-default.resizable:	true
     XTerm*configPopup.frame.ip.sl-default.label:		WhenNoMatch

!      { (simpleMenu)  frame.ip.sl-default.menu

!        { (smeBSB)  frame.ip.sl-default.menu.always
         XTerm*configPopup.frame.ip.sl-default.menu.always.label:	Always
!        } (smeBSB)

!        { (smeBSB)  frame.ip.sl-default.menu.never
         XTerm*configPopup.frame.ip.sl-default.menu.never.label:	Never
!        } (smeBSB)

!        { (smeBSB)  frame.ip.sl-default.menu.whennomatch
         XTerm*configPopup.frame.ip.sl-default.menu.whennomatch.label:	WhenNoMatch
!        } (smeBSB)
!      } (simpleMenu)
!    } (menuButton)

!    { (menuButton)  frame.ip.sg-default
     XTerm*configPopup.frame.ip.sg-default.resizable:	true
     XTerm*configPopup.frame.ip.sg-default.label:		Yes

!      { (simpleMenu)  frame.ip.sg-default.menu

!        { (smeBSB)  frame.ip.sg-default.menu.yes
         XTerm*configPopup.frame.ip.sg-default.menu.yes.label:	Yes
!        } (smeBSB)

!        { (smeBSB)  frame.ip.sg-default.menu.no
         XTerm*configPopup.frame.ip.sg-default.menu.no.label:	No
!        } (smeBSB)
!      } (simpleMenu)
!    } (menuButton)

!    { (menuButton)  frame.ip.as-default
     XTerm*configPopup.frame.ip.as-default.resizable:	true
     XTerm*configPopup.frame.ip.as-default.label:		Yes

!      { (simpleMenu)  frame.ip.as-default.menu

!        { (smeBSB)  frame.ip.as-default.menu.yes
         XTerm*configPopup.frame.ip.as-default.menu.yes.label:	Yes
!        } (smeBSB)

!        { (smeBSB)  frame.ip.as-default.menu.no
         XTerm*configPopup.frame.ip.as-default.menu.no.label:	No
!        } (smeBSB)
!      } (simpleMenu)
!    } (menuButton)

!    { (command)  frame.ip.apply
     XTerm*configPopup.frame.ip.apply.label:		apply
!    } (command)
!  } (form)

!  { (box)  frame.bottom_bx

!    { (command)  frame.bottom_bx.done
     XTerm*configPopup.frame.bottom_bx.done.label:	DONE
!    } (command)
!  } (box)
!} (paned)
!
! END_CXTERM_RESOURCE
