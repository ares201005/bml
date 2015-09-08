/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Copyright 2010.  Los Alamos National Security, LLC. This material was    !
! produced under U.S. Government contract DE-AC52-06NA25396 for Los Alamos !
! National Laboratory (LANL), which is operated by Los Alamos National     !
! Security, LLC for the U.S. Department of Energy. The U.S. Government has !
! rights to use, reproduce, and distribute this software.  NEITHER THE     !
! GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY,     !
! EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS         !
! SOFTWARE.  If software is modified to produce derivative works, such     !
! modified software should be clearly marked, so as not to confuse it      !
! with the version available from LANL.                                    !
!                                                                          !
! Additionally, this program is free software; you can redistribute it     !
! and/or modify it under the terms of the GNU General Public License as    !
! published by the Free Software Foundation; version 2.0 of the License.   !
! Accordingly, this program is distributed in the hope that it will be     !
! useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   !
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General !
! Public License for more details.                                         !
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <math.h>
#include <stdio.h>

#include "Matrix.h"

extern int ndevices;
extern int nblocks;
extern int nstreams;

extern "C" void sp2pure_seq_nospin3(int hdim, REAL *x0_pointer, REAL *vvx, 
                         int *jjp, int numiter, int ncorerows, int *pp, 
                         int idevice, int istream) {

  REAL trx, scalar;
  Matrix xtmp, x0;
  Vector xjjp;
  
  M_Init(xtmp, hdim, hdim);
  M_InitWithLocal(x0, x0_pointer, hdim, hdim);
  M_InitWithLocalVector(xjjp, jjp, ncorerows);

  // Copy x0 to all GPUs  
  M_PushAsync( x0, idevice, istream );
  M_PushVectorAsync( xjjp, idevice, istream );

  // SP2 loop using sequence
  for (int i = 0; i < numiter; i++) {
    
    // On multiple GPUs: xtmp = x0 - x0 * x0

    //M_CopyMgpu(x0, xtmp);
    M_CopyAsync(x0, xtmp, idevice, istream);

    //M_MultiplyMgpuStrm( &MINUS1, x0, x0, &ONE, xtmp, istream);
    M_MultiplyAsync( &MINUS1, x0, x0, &ONE, xtmp, idevice, istream);

    scalar = ONE - TWO * (REAL)pp[i];
    M_MultiplyScalarSumAsync( &scalar, xtmp, x0, idevice, istream);

    // Calculate partial trace trace[(X-X^2]^2]
    trx = M_PartialTraceMgpuStrm( xtmp, xjjp, ncorerows, istream );
    //trx = M_PartialTraceAsync( xtmp, device_jjp, ncorerows, istream );
    vvx[i] += trx;

    // Distribute x0
    //if (ndevices > 1) M_CollectDistributeMgpu(x0);

  }
    
  //M_PullMgpu(x0);
  M_PullAsync(x0, idevice, istream);
  
  M_DeallocateLocal(xtmp);
  M_DeallocateDevice(xtmp);
  M_DeallocateDeviceVector(xjjp);
  M_DeallocateDevice(x0);

}

//
// sp2pure_seq_spin3 - This has NOT been tested.
//
void sp2pure_seq_spin3(int  hdim, REAL *rhoup_pointer, REAL *rhodown_pointer, 
                       REAL *vvx, Vector jjp, int numiter, int ncorerows, int *pp,
                       int istream) {

  REAL trx;

  Matrix xtmpup, xtmpdown, rhoup, rhodown;

  M_Init(xtmpup, hdim, hdim);
  M_Init(xtmpdown, hdim, hdim);
  M_InitWithLocal(rhoup, rhoup_pointer, hdim, hdim);
  M_InitWithLocal(rhodown, rhodown_pointer, hdim, hdim);
  
  M_PushMgpu(rhoup);
  M_PushMgpu(rhodown);

  for (int i = 0; i < numiter; i++) {

    M_CopyMgpu(rhoup, xtmpup);
    M_CopyMgpu(rhodown, xtmpdown);

    M_MultiplyMgpu( &MINUS1, rhoup, rhoup, &ONE, xtmpup);
    M_MultiplyMgpu( &MINUS1, rhodown, rhodown, &ONE, xtmpdown);
    
    trx = M_PartialTraceMgpuStrm( xtmpup, jjp, ncorerows, istream ) + 
          M_PartialTraceMgpuStrm( xtmpdown, jjp, ncorerows, istream );
    vvx[i] += trx;
    
    if (pp[i] == 0 ) {
      
      // rhoup = rhoup + xtmpup
      // rhodown = rhodown + xtmpdown
      M_MultiplyScalarSumMgpu( &ONE, xtmpup, rhoup);
      M_MultiplyScalarSumMgpu( &ONE, xtmpdown, rhodown);
      
    } 
    else {

      // rhoup = rhoup - xtmpup
      // rhodown = rhodown - xtmpdown
      M_MultiplyScalarSumMgpu( &MINUS1, xtmpup, rhoup);
      M_MultiplyScalarSumMgpu( &MINUS1, xtmpdown, rhodown);
      
    }
    
    // Distribute
    if (ndevices > 1) {
      M_CollectDistributeMgpu(rhoup);
      M_CollectDistributeMgpu(rhodown);
    }
  }

  M_PullMgpu(rhoup);
  M_PullMgpu(rhodown);

  M_DeallocateLocal(xtmpup);
  M_DeallocateDevice(xtmpup);
  M_DeallocateLocal(xtmpdown);
  M_DeallocateDevice(xtmpdown);
  M_DeallocateDevice(rhoup);
  M_DeallocateDevice(rhodown);

}
