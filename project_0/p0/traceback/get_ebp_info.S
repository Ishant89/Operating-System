/** @file get_ebp_info.S 
 *  @brief Get %ebp register info function
 *
 *  This file contains assembly instructions to
 *  fetch the value of %ebp register when traceback
 *  function (from traceback library is called
 *
 *  @author Ishant Dawer(idawer)
 *  @bug No known bugs
 */

/* define the add_one1 and add_one2 labels so that they can be called from
 *  * other files (.c or .S) */

.global get_ebp

/** @brief Get ebp register value
 *
 * This function gets the latest ebp register value
 * from the stack 
 */

get_ebp:
        movl %ebp,%eax
        ret
