# Failure Scene Capture

To effectively capture failure scene, we propose a considerate method that combines content-aware memory image pruning with failsafe data collection.
The source code we provide below is based on [xCrash](https://github.com/iqiyi/xCrash), an open-source failure capture tool for Android.

## Memory Image Pruning and Recording
Compared to existing tools, we additionally record the in-situ memory data to realize comprehensive data capturing. Specifically, instead of directly `coredump`ing the full image of the process memory, we carefully prune the image to reduce the excessive storage overhead.
The image-pruning mechanism is implemented in `fc_coredump_memory()`.

```c++
#define CHECK_MAPS(_maps)
    do{
        if (NULL != strstr(map->name, _maps))
            return 0;
    } while(0)
// prune the image by removing redundant and non-critical data
static size_t dump_size(fc_map_t* map, int java_dump){
    // ignore no permission segment
    if (!(map->flags & PROT_READ) && !(map->flags & PROT_WRITE)){
        return 0;
    }
    if (map->name != NULL){
        if (map->flags & PROT_EXEC){
            return 0;
        }
        if (!map->dump_state){
            CHECK_SUFFIX(".so");
        }
        CHECK_MAPS(".db");
        CHECK_MAPS(".crc");
        CHECK_MAPS(".hyb");
        CHECK_MAPS(".dat");
        CHECK_MAPS(".crc");
        CHECK_MAPS(".hyb");
        CHECK_MAPS(".ttf");
        CHECK_MAPS(".lock");
        CHECK_MAPS(".relro");
        CHECK_MAPS(".db-shm");
        CHECK_MAPS(".data");
        CHECK_MAPS(".otf");
        CHECK_MAPS("anon_inode:dmabuf");
        CHECK_MAPS("Cookies");
        CHECK_MAPS("[vectors]");
        CHECK_MAPS("event-log-tags");
        CHECK_MAPS("settings_config");
        CHECK_MAPS("thread signal stack");
        if (!java_dump){
            CHECK_MAPS("jit-cache");
            CHECK_MAPS(".art");
            CHECK_MAPS(".oat");
            CHECK_MAPS(".vdex");
            CHECK_MAPS(".odex");
            CHECK_MAPS(".dex");
            CHECK_MAPS(".apk");
            CHECK_MAPS(".jar");
            CHECK_MAPS("/data/dalvik-cache");
            CHECK_MAPS("/dev/ashmem");
            CHECK_MAPS("/dev/__properties__");
            CHECK_MAPS("anon:dalvik");
            CHECK_MAPS("jit-cache");
        }
    }
    else if (!(map->flags & PROT_WRITE)){
        return 0;
    }
    return map->end - map->start;
}
/*
 * coredump the process memory image.
 * 'xcd_maps_t' and 'xcd_maps_itme_t' are defined in 'xcd_maps.c' (whose location in xCrash tree is 'xcrash_lib/src/main/cpp/xcrash_dumper/xcd_maps.c')
*/
void fc_coredump_memory(xcd_maps_t *self, int fd){
    xcd_maps_itme_t *mi;
    ElfW(Phdr) phdr;
    uintptr_t  size = 0;
    TAILQ_FOREACH (mi, &(self->maps), link)
    {
        phdr.p_type = PT_LOAD;
        phdr.p_offset = offest;
        phdr.p_vaddr = mi->maps.start;
        phdr.p_paddr = 0;
        phdr.p_filesz = dump_size(&mi->map);
        phdr.p_memsz = mi->map.end - mi->map.start;
        offset += phdr.p_filesz;
        phdr.p_flags = m->map.flags & PROT_READ ? PF_R : 0;
        if (m->map.flags & PROT_WRITE){
            phdr.p_flags |= PF_W;
        }
        if (m->map.flags & PROT_EXEC){
            phdr.p_flags |= PF_X;
        }
        phdr.p_align = PAGE_SIZE;
        nwrite = XCC_UTIL_TEMP_FAILURE_RETRY(write(fd, &phdr, sizeof(phdr)));
        if nwrite != sizeof(phdr){
            LOGE("FC: coredump error!");
            return;
        }
        size += sizeof(phdr);
    }
    if(0 != xcc_util_write_format(fd, "    TOTAL SIZE: 0x%"PRIxPTR"K (%"PRIuPTR"K)\n\n",
                                size / 1024, size / 1024)){
        LOGE("FC: coredump total size record error!");
        return;
    }
}
```

##  Failsafe Data Collection
In addition, the data collection are excuted in four dedicated processes instead of a single one for failsafe. Our modifications mainly involve the `xcd_process_record` function in [`xcd_process.c`](https://github.com/iqiyi/xCrash/blob/master/xcrash_lib/src/main/cpp/xcrash_dumper/xcd_process.c#L389).

```diff
int xcd_process_record(xcd_process_t *self,
                       int log_fd,
                       unsigned int logcat_system_lines,
                       unsigned int logcat_events_lines,
                       unsigned int logcat_main_lines,
                       int dump_elf_hash,
                       int dump_map,
                       int dump_fds,
                       int dump_network_info,
                       int dump_all_threads,
                       unsigned int dump_all_threads_count_max,
                       char *dump_all_threads_whitelist,
                       int api_level)
{
...    
    TAILQ_FOREACH(thd, &(self->thds), link)
    {
        if(thd->t.tid == self->crash_tid)
        {
-            if(0 != (r = xcd_thread_record_info(&(thd->t), log_fd, self->pname))) return r;
-            if(0 != (r = xcd_process_record_signal_info(self, log_fd))) return r;
-            if(0 != (r = xcd_process_record_abort_message(self, log_fd, api_level))) return r;
-            if(0 != (r = xcd_thread_record_regs(&(thd->t), log_fd))) return r;
-            if(0 == xcd_thread_load_frames(&(thd->t), self->maps))           
-            {
-                if(0 != (r = xcd_thread_record_backtrace(&(thd->t), log_fd))) return r;
-                if(0 != (r = xcd_thread_record_buildid(&(thd->t), log_fd, dump_elf_hash, 
-                          xcc_util_signal_has_si_addr(self->si) ? -(uintptr_t)self->si->si_addr : 0))) return r;
-                if(0 != (r = xcd_thread_record_stack(&(thd->t), log_fd))) return r;
-                if(0 != (r = xcd_thread_record_memory(&(thd->t), log_fd))) return r;
-            }
-            if(dump_map) if(0 != (r = xcd_maps_record(self->maps, log_fd))) return r;
-            if(0 != (r = xcc_util_record_logcat(log_fd, self->pid, api_level, 
-                        logcat_system_lines, logcat_events_lines,logcat_main_lines))) return r;
-            if(dump_fds) if(0 != (r = xcc_util_record_fds(log_fd, self->pid))) return r;
-            if(dump_network_info) if(0 != (r = xcc_util_record_network_info(log_fd, self->pid, api_level))) return r;
-            if(0 != (r = xcc_meminfo_record(log_fd, self->pid))) return r;
+            // record execution contexts
+            pid_t context_pid = fork(); 
+            if(-1 == context_pid)
+            {
+                xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"excution context fork failed");
+                return -1;
+            }
+            else if(0 == context_pid)
+            {
+                if(0 != (r = xcd_thread_record_info(&(thd->t), log_fd, self->pname))) goto err_context;
+                if(0 != (r = xcd_process_record_signal_info(self, log_fd))) goto err_context;
+                if(0 != (r = xcd_process_record_abort_message(self, log_fd, api_level))) goto err_context;
+                if(0 != (r = xcd_thread_record_regs(&(thd->t), log_fd))) goto err_context;
+                if(0 == xcd_thread_load_frames(&(thd->t), self->maps))           
+                {
+                    if(0 != (r = xcd_thread_record_backtrace(&(thd->t), log_fd))) goto err_context;
+                    if(0 != (r = xcd_thread_record_buildid(&(thd->t), log_fd, dump_elf_hash, 
+                              xcc_util_signal_has_si_addr(self->si) ? -(uintptr_t)self->si->si_addr : 0))) goto err_context;
+                    if(0 != (r = xcd_thread_record_stack(&(thd->t), log_fd))) goto err_context;
+                    if(0 != (r = xcd_thread_record_memory(&(thd->t), log_fd))) goto err_context;                       
+                }
+                exit(0);
+                err_context:
+                    xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"excution context record failed");
+                    return;
+            }
+            // record the memory image 
+            pid_t image_pid = fork();
+            if(-1 == image_pid)
+            {
+                xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"memory image fork failed");
+                return -1;
+            }
+            else if(0 == image_pid)
+            {
+                //the memory image recording function mentioned above
+                if(dump_map) if(0 != (r = fc_coredump_memory(self->maps, log_fd))) goto err_image; 
+                exit(0);
+                err_image:
+                    xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"memory image record failed");
+                    return;
+            }
+            // record Android logcat
+            pid_t logcat_pid = fork();
+            if(-1 == logcat_pid)
+            {
+                xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"Android logcat fork failed");
+                return -1;
+            }
+            else if(0 == logcat_pid)
+            {
+                if(0 != (r = xcc_util_record_logcat(log_fd, self->pid, api_level, 
+                        logcat_system_lines, logcat_events_lines,logcat_main_lines))) goto err_logcat; 
+                exit(0);
+                err_logcat:
+                    xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"Android logcat record failed");
+                    return;
+            }
+            // record system resources
+            pid_t resource_pid = fork();
+            if(-1 == resource_pid)
+            {
+                xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"system resources fork failed");
+                return -1;
+            }
+            else if(0 == resource_pid)
+            {
+                if(dump_fds) if(0 != (r = xcc_util_record_fds(log_fd, self->pid))) goto err_resource;
+                if(dump_network_info) if(0 != (r = xcc_util_record_network_info(log_fd, self->pid, api_level))) goto err_resource;
+                if(0 != (r = xcc_meminfo_record(log_fd, self->pid))) goto err_resource;
+                exit(0);
+                err_resource:
+                    xcc_util_write_format_safe(log_fd, XC_CRASH_ERR_TITLE"system resources record failed");
+                    return;
+            }
            break;
        }
    }
...
}
```