/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include "apr.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_signal.h"

#include "apr_arch_misc.h"       /* for WSAHighByte / WSALowByte */
#include "apr_arch_proc_mutex.h" /* for apr_proc_mutex_unix_setup_lock() */
#include "apr_arch_internal_time.h"


APR_DECLARE(apr_status_t) apr_app_initialize(int *argc, 
                                             const char * const * *argv, 
                                             const char * const * *env)
{
    /* An absolute noop.  At present, only Win32 requires this stub, but it's
     * required in order to move command arguments passed through the service
     * control manager into the process, and it's required to fix the char*
     * data passed in from win32 unicode into utf-8, win32's apr internal fmt.
     */
    return apr_initialize();
}

APR_DECLARE(apr_status_t) apr_initialize(void)
{
    apr_pool_t *pool;
    apr_status_t status;
    int iVersionRequested;
    WSADATA wsaData;
    int err;

    /* Register the NLM as using APR. If it is already
        registered then just return. */
    if (register_NLM(getnlmhandle()) != 0) {
        return APR_SUCCESS;
    }

    /* apr_pool_initialize() is being called from the library
        startup code since all of the memory resources belong 
        to the library rather than the application. */
    
    if (apr_pool_create(&pool, NULL) != APR_SUCCESS) {
        return APR_ENOPOOL;
    }

    apr_pool_tag(pool, "apr_initilialize");

    iVersionRequested = MAKEWORD(WSAHighByte, WSALowByte);
    err = WSAStartup((WORD) iVersionRequested, &wsaData);
    if (err) {
        return err;
    }
    if (LOBYTE(wsaData.wVersion) != WSAHighByte ||
        HIBYTE(wsaData.wVersion) != WSALowByte) {
        WSACleanup();
        return APR_EEXIST;
    }
    
    apr_signal_init(pool);
//    setGlobalPool((void*)pool);

    return APR_SUCCESS;
}

APR_DECLARE_NONSTD(void) apr_terminate(void)
{
    /* Unregister the NLM. If it is not registered
        then just return. */
    if (unregister_NLM(getnlmhandle()) != 0) {
        return;
    }

    /* apr_pool_terminate() is being called from the 
        library shutdown code since the memory resources
        belong to the library rather than the application */

    /* Just clean up the memory for the app that is going
        away. */
    netware_pool_proc_cleanup ();
    WSACleanup();
}

APR_DECLARE(void) apr_terminate2(void)
{
    apr_terminate();
}
