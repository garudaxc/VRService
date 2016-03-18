#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>


inline size_t roundUpToPageSize(size_t x) {
    return (x + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}

static void log_fscreeninfo(const struct fb_fix_screeninfo* finfo)
{
	printf("fixscreen info:\n");

	printf("id %s\n",finfo->id);       //s3c2410fb
	printf("smem_start %u\n",(unsigned int)finfo->smem_start); //帧缓冲 内存开始地址,物理地址
	printf("smem_len %d\n",finfo->smem_len); // 帧缓冲 内存 长度
	printf("type %d\n",finfo->type);
	printf("type_aux %d\n",finfo->type_aux);//平面交织交替
	printf("visual %d\n",finfo->visual); //记录 色彩模式   2
	printf("xpanstep %d\n",finfo->xpanstep);//如果没有硬件panning，赋0
	printf("ypanstep %d\n",finfo->ypanstep);
	printf("line_length %d\n",finfo->line_length);  //  640
	printf("mmio_start %u\n",(unsigned int)finfo->mmio_start);//内存映射IO开始地址 物理地址
	printf("mmio_len %d\n",finfo->mmio_len);//内存映射IO 长度
	printf("accel %d\n\n",finfo->accel);
}

static void log_vscreeninfo(const struct fb_var_screeninfo* vinfo)
{
    printf("log_vscreeninfo info:\n");
	printf("xres %d\n",vinfo->xres); //可见解析度  320
    printf("yres %d\n",vinfo->yres);          //  240
    printf("xres_virturl %d\n",vinfo->xres_virtual); //虚拟解析度  320
    printf("yres_virtual %d\n",vinfo->yres_virtual);           //   240
    printf("xoffset %d\n",vinfo->xoffset); //虚拟到可见的偏移        0
    printf("yoffset %d\n",vinfo->yoffset);                     //    0
    printf("bits_per_pixel %d\n",vinfo->bits_per_pixel); //每像素位数 bpp  16
    printf("grayscale %d\n",vinfo->grayscale);//非零时，指灰度

    printf("fb_bitfield red.offset %d\n",vinfo->red.offset);     //11  偏移11位
    printf("fb_bitfield .length %d\n",vinfo->red.length);        // 5
    printf("fb_bitfield .msb_right %d\n",vinfo->red.msb_right);  //  0
    printf("fb_bitfield green.offset %d\n",vinfo->green.offset); // 5 偏移5位
    printf("fb_bitfield .length %d\n",vinfo->green.length);       // 6
    printf("fb_bitfield .msb_right %d\n",vinfo->green.msb_right); // 0
    printf("fb_bitfield blue.offset %d\n",vinfo->blue.offset);
    printf("fb_bitfield .length %d\n",vinfo->blue.length);
    printf("fb_bitfield .msb_right %d\n",vinfo->blue.msb_right);
    printf("fb_bitfield transp.offset %d\n",vinfo->transp.offset);
    printf("fb_bitfield .length %d\n",vinfo->transp.length);
    printf("fb_bitfield .msb_right %d\n",vinfo->transp.msb_right);

    printf("nonstd %d\n",vinfo->nonstd); //!=0 非标准像素格式
    printf("activate %d\n",vinfo->activate);
    printf("height %d\n",vinfo->height); //高度/  240
    printf("widht %d\n",vinfo->width);          //   320
    printf("accel_flags %d\n",vinfo->accel_flags); //看 fb_info.flags

    //定时，除了 pixclock之外，其他的都以像素时钟为单位
    printf("pixclock %d\n",vinfo->pixclock);//像素时钟，皮秒   80000
    printf("left_margin %d\n",vinfo->left_margin);//行切换：从同步到绘图之间的延迟    28
    printf("right_margin %d\n",vinfo->right_margin);//行切换：从绘图到同步之间的延迟   24
    printf("upper_margin %d\n",vinfo->upper_margin);//帧切换：从同步到绘图之间的延迟   6
    printf("lower_margin %d\n",vinfo->lower_margin);//帧切换：从绘图到同步之间的延迟    2

    printf("hsync_len %d\n",vinfo->hsync_len); //hor 水平同步的长度         42
    printf("vsync_len %d\n",vinfo->vsync_len); //vir 垂直同步的长度         12

    printf("sync %d\n",vinfo->sync); //
    printf("vmode %d\n",vinfo->vmode);
    printf("rotate %d\n",vinfo->rotate);
}


#define MDP_DISPLAY_COMMIT_OVERLAY 0x00000001

struct mdp_display_commit {
    uint32_t flags;
    uint32_t wait_for_finish;
    struct fb_var_screeninfo var;
};

#define MSMFB_IOCTL_MAGIC 'm'
#define MSMFB_DISPLAY_COMMIT _IOW(MSMFB_IOCTL_MAGIC, 164, struct mdp_display_commit)


int FramebufferTest()
{
    char const * const device_template[] = {
            "/dev/graphics/fb%u",
            "/dev/fb%u",
            0 };

    int fd = -1;
    int i=0;
    char name[64];
    char property[PROPERTY_VALUE_MAX];

    while ((fd==-1) && device_template[i]) {
        snprintf(name, 64, device_template[i], 0);
        fd = open(name, O_RDWR, 0);
        i++;
    }
    if (fd < 0){
        printf("can not open devcies fb\n");
        return -errno;
    }

//    memset(&module->commit, 0, sizeof(struct mdp_display_commit));
//
    struct fb_fix_screeninfo finfo;
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1){
        printf("can not FBIOGET_FSCREENINFO %d\n", errno);
        return -errno;
    }
    log_fscreeninfo(&finfo);

    struct fb_var_screeninfo info;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1){
        printf("can not FBIOGET_VSCREENINFO %d\n", errno);
        return -errno;
    }

    log_vscreeninfo(&info);

    int fbSize = 1920 * 1080 * 4 + 1920 * 200 * 4;
    fbSize = roundUpToPageSize(fbSize);
    void* vaddr = mmap(0, fbSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (vaddr == NULL) {
        printf("mmap failed!\n");
        return -1;
    }

    memset(vaddr, 0, fbSize);
    printf("map addr %p\n", vaddr);

    return 0;
}


#define MIN_HWC_HEADER_VERSION HWC_HEADER_VERSION


static uint32_t hwcApiVersion(const hwc_composer_device_1_t* hwc) {
    uint32_t hwcVersion = hwc->common.version;
    return hwcVersion & HARDWARE_API_VERSION_2_MAJ_MIN_MASK;
}

static uint32_t hwcHeaderVersion(const hwc_composer_device_1_t* hwc) {
    uint32_t hwcVersion = hwc->common.version;
    return hwcVersion & HARDWARE_API_VERSION_2_HEADER_MASK;
}

static bool hwcHasApiVersion(const hwc_composer_device_1_t* hwc,
                             uint32_t version) {
    return hwcApiVersion(hwc) >= (version & HARDWARE_API_VERSION_2_MAJ_MIN_MASK);
}

framebuffer_device_t*           mFbDev;
struct hwc_composer_device_1*   mHwc;

#define ALOGE printf


int loadFbHalModule()
{
    hw_module_t const* module;

    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    if (err != 0) {
        ALOGE("%s module not found", GRALLOC_HARDWARE_MODULE_ID);
        return err;
    }

    return framebuffer_open(module, &mFbDev);
}


// Load and prepare the hardware composer module.  Sets mHwc.
void loadHwcModule()
{
    hw_module_t const* module;

    if (hw_get_module(HWC_HARDWARE_MODULE_ID, &module) != 0) {
        ALOGE("%s module not found", HWC_HARDWARE_MODULE_ID);
        return;
    }

    int err = hwc_open_1(module, &mHwc);
    if (err) {
        ALOGE("%s device failed to initialize (%s)",
              HWC_HARDWARE_COMPOSER, strerror(-err));
        return;
    }

    if (!hwcHasApiVersion(mHwc, HWC_DEVICE_API_VERSION_1_0) ||
        hwcHeaderVersion(mHwc) < MIN_HWC_HEADER_VERSION ||
        hwcHeaderVersion(mHwc) > HWC_HEADER_VERSION) {
        ALOGE("%s device version %#x unsupported, will not be used",
              HWC_HARDWARE_COMPOSER, mHwc->common.version);
        hwc_close_1(mHwc);
        mHwc = NULL;
        return;
    }
}



int main(int argc, char **argv) {
    printf("fbtest !! \n");

    loadFbHalModule();
    loadHwcModule();

    printf("%p %p\n", mFbDev, mHwc);

//    info.activate = FB_ACTIVATE_VBL;
//    info.yoffset = 0;
//    if (ioctl(fd, FBIOPUT_VSCREENINFO, &info) == -1) {
//        printf("%s: FBIOPUT_VSCREENINFO for primary failed, str: %s\n",
//              __FUNCTION__, strerror(errno));
//        return -errno;
//    }
//    printf("FBIOPUT_VSCREENINFO\n");

//    i = 50;
//    while (i--) {
//
//        struct mdp_display_commit commit_info;
//        memset(&commit_info, 0, sizeof(struct mdp_display_commit));
//        commit_info.flags = MDP_DISPLAY_COMMIT_OVERLAY;
//        if(ioctl(fd, MSMFB_DISPLAY_COMMIT, &commit_info) == -1) {
//            printf("%s: MSMFB_DISPLAY_COMMIT for primary failed\n", __FUNCTION__);
//            return -errno;
//        }
//
//        printf("MSMFB_DISPLAY_COMMIT\n");
//    }


    return 0;
}