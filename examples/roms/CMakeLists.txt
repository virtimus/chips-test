fips_begin_lib(roms)
    fipsutil_embed(atom-roms.yml atom-roms.h)
    fipsutil_embed(c64-roms.yml c64-roms.h)
    fipsutil_embed(cpc-roms.yml cpc-roms.h)
    fipsutil_embed(kc85-roms.yml kc85-roms.h)
    fipsutil_embed(z1013-roms.yml z1013-roms.h)
    fipsutil_embed(z9001-roms.yml z9001-roms.h)
    fipsutil_embed(zx-roms.yml zx-roms.h)
    fipsutil_embed(bombjack-roms.yml bombjack-roms.h)
    fipsutil_embed(pacman-roms.yml pacman-roms.h)
    fipsutil_embed(pengo-roms.yml pengo-roms.h)
    fipsutil_embed(lc80-roms.yml lc80-roms.h)
    fipsutil_embed(cp4-roms.yml cp4-roms.h)
    fipsutil_embed(c1541-roms.yml c1541-roms.h)
    fipsutil_embed(vic20-roms.yml vic20-roms.h)
    fips_files(dummy.c)
fips_end_lib()
