/** @file get_ebp_info.S 
 *  @brief Get %ebp register info function
 *
 *  @author Ishant Dawer(idawer)
 */

/* define the add_one1 and add_one2 labels so that they can be called from
 *  * other files (.c or .S) */

.global get_ebp

/* 
 * Method 1 helps in getting the %ebp register info when called from 
 * traceback function 
 */

get_ebp:
        movl %ebp,%eax
        ret
