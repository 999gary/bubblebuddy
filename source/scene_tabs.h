#ifndef SCENE_TABS_H
#define SCENE_TABS_H

typedef struct { u32 id; void *table; int count; } scene_table_meta;
#ifdef BFBBMIX
const scene_table_meta scene_tabs[] = {
    {FOURCC_HB01, (void *)HB01_table, ArrayCount(HB01_table)},
};
#else
const scene_table_meta scene_tabs[] = {
    {FOURCC_JF01, (void *)JF01_table, ArrayCount(JF01_table)},
    {FOURCC_JF02, (void *)JF02_table, ArrayCount(JF02_table)},
    {FOURCC_JF03, (void *)JF03_table, ArrayCount(JF03_table)},
    {FOURCC_JF04, (void *)JF04_table, ArrayCount(JF04_table)},
    {FOURCC_KF01, (void *)KF01_table, ArrayCount(KF01_table)},
    {FOURCC_KF02, (void *)KF02_table, ArrayCount(KF02_table)},
    {FOURCC_KF04, (void *)KF04_table, ArrayCount(KF04_table)},
    {FOURCC_KF05, (void *)KF05_table, ArrayCount(KF05_table)},
    {FOURCC_MNU3, (void *)MNU3_table, ArrayCount(MNU3_table)},
    {FOURCC_RB01, (void *)RB01_table, ArrayCount(RB01_table)},
    {FOURCC_RB02, (void *)RB02_table, ArrayCount(RB02_table)},
    {FOURCC_RB03, (void *)RB03_table, ArrayCount(RB03_table)},
    {FOURCC_SM01, (void *)SM01_table, ArrayCount(SM01_table)},
    {FOURCC_SM02, (void *)SM02_table, ArrayCount(SM02_table)},
    {FOURCC_SM03, (void *)SM03_table, ArrayCount(SM03_table)},
    {FOURCC_SM04, (void *)SM04_table, ArrayCount(SM04_table)},
    {FOURCC_B101, (void *)B101_table, ArrayCount(B101_table)},
    {FOURCC_B201, (void *)B201_table, ArrayCount(B201_table)},
    {FOURCC_B302, (void *)B302_table, ArrayCount(B302_table)},
    {FOURCC_B303, (void *)B303_table, ArrayCount(B303_table)},
    {FOURCC_BB01, (void *)BB01_table, ArrayCount(BB01_table)},
    {FOURCC_BB02, (void *)BB02_table, ArrayCount(BB02_table)},
    {FOURCC_BB03, (void *)BB03_table, ArrayCount(BB03_table)},
    {FOURCC_BB04, (void *)BB04_table, ArrayCount(BB04_table)},
    {FOURCC_BC01, (void *)BC01_table, ArrayCount(BC01_table)},
    {FOURCC_BC02, (void *)BC02_table, ArrayCount(BC02_table)},
    {FOURCC_BC03, (void *)BC03_table, ArrayCount(BC03_table)},
    {FOURCC_BC04, (void *)BC04_table, ArrayCount(BC04_table)},
    {FOURCC_BC05, (void *)BC05_table, ArrayCount(BC05_table)},
    {FOURCC_DB01, (void *)DB01_table, ArrayCount(DB01_table)},
    {FOURCC_DB02, (void *)DB02_table, ArrayCount(DB02_table)},
    {FOURCC_DB03, (void *)DB03_table, ArrayCount(DB03_table)},
    {FOURCC_DB04, (void *)DB04_table, ArrayCount(DB04_table)},
    {FOURCC_DB06, (void *)DB06_table, ArrayCount(DB06_table)},
    {FOURCC_GL01, (void *)GL01_table, ArrayCount(GL01_table)},
    {FOURCC_GL02, (void *)GL02_table, ArrayCount(GL02_table)},
    {FOURCC_GL03, (void *)GL03_table, ArrayCount(GL03_table)},
    {FOURCC_GY01, (void *)GY01_table, ArrayCount(GY01_table)},
    {FOURCC_GY02, (void *)GY02_table, ArrayCount(GY02_table)},
    {FOURCC_GY03, (void *)GY03_table, ArrayCount(GY03_table)},
    {FOURCC_GY04, (void *)GY04_table, ArrayCount(GY04_table)},
    {FOURCC_HB00, (void *)HB00_table, ArrayCount(HB00_table)},
    {FOURCC_HB01, (void *)HB01_table, ArrayCount(HB01_table)},
    {FOURCC_HB02, (void *)HB02_table, ArrayCount(HB02_table)},
    {FOURCC_HB03, (void *)HB03_table, ArrayCount(HB03_table)},
    {FOURCC_HB04, (void *)HB04_table, ArrayCount(HB04_table)},
    {FOURCC_HB05, (void *)HB05_table, ArrayCount(HB05_table)},
    {FOURCC_HB06, (void *)HB06_table, ArrayCount(HB06_table)},
    {FOURCC_HB07, (void *)HB07_table, ArrayCount(HB07_table)},
    {FOURCC_HB08, (void *)HB08_table, ArrayCount(HB08_table)},
    {FOURCC_HB09, (void *)HB09_table, ArrayCount(HB09_table)},
    {FOURCC_HB10, (void *)HB10_table, ArrayCount(HB10_table)},
    {FOURCC_PG12, (void *)PG12_table, ArrayCount(PG12_table)},
};
#endif

#endif //SCENE_TABS_H
