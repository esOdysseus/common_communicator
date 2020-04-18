/***
 * protocol.c
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>

#define LOGGER_TAG "PAL"
#include <logger.h>
#include <pal/protocol.h>

/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the pmi.
 * @return 0 = success, !0 = failure.
 */
static int load(const char *id,
        const char *path,
        const struct pt_module_t **pPmi)
{
    int status = -EINVAL;
    void *handle = NULL;
    struct pt_multi_module_t *fpmi = NULL;
    struct pt_module_t *pmi = NULL;
    /*
     * load the symbols resolving undefined symbols before
     * dlopen returns. Since RTLD_GLOBAL is not or'd in with
     * RTLD_NOW the external symbols will not be global
     */
    handle = dlopen(path, RTLD_NOW);
    if (handle == NULL) {
        char const *err_str = dlerror();
        LOGERR("load: module=%s\n%s\n", path, err_str?err_str:"unknown");
        status = -EINVAL;
        goto done;
    }
    /* Get the address of the struct hal_module_info. */
    const char *sym = PAL_MODULE_INFO_SYM_AS_STR;
    fpmi = (struct pt_multi_module_t *)dlsym(handle, sym);
    if (fpmi == NULL) {
        LOGERR("load: couldn't find symbol %s\n", sym);
        status = -EINVAL;
        goto done;
    }
    /* Check that the id matches */
    for(unsigned int i=0; i < fpmi->mod_cnt; i++) {
    	pmi = &(fpmi->module[i]);
    	if (pmi == NULL)
    		continue;

    	if (strcmp(id, pmi->id) == 0)
    		break;
    	pmi = NULL;
    }
    if (pmi == NULL) {
		LOGERR("load: id=%s != pmi->id=%s\n", id, pmi->id);
		status = -EINVAL;
		goto done;
	}
    pmi->dso = handle;
    /* success */
    status = 0;
done:
    if (status != 0) {
        pmi = NULL;
        if (handle != NULL) {
            dlclose(handle);
            handle = NULL;
        }
    } else {
        LOGI("loaded PAL id=%s path=%s pmi=%p handle=%p\n",
                id, path, pmi, handle);
    }
    *pPmi = pmi;
    return status;
}


int pt_get_module(const char *id, const char *path, const struct pt_module_t **module)
{
    return load(id, path, module);
}


