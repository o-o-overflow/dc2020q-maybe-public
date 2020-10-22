all:
	wcl386 maybe.c
	sed -i 's/main_/niam_/' maybe
	sed -i 's/Open Watcom C\/C++32 Run-Time system. Copyright (c) Open Watcom Contributors 2002-2020. Portions Copyright (C) Sybase, Inc. 1988-2002/                                                                                                                                    /' maybe

deploy:
	cp maybe ../public/maybe
	cp vault.bf ../public/vault.bf
