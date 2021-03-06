/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 The Boeing Company
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Gary Pei <guangyu.pei@boeing.com>
 */

#include <cmath>
#include "nist-error-rate-model.h"

NistErrorRateModel::NistErrorRateModel()
{
}

NistErrorRateModel::~NistErrorRateModel()
{
}

double NistErrorRateModel::GetBpskBer(double snr) const
{
    double z = std::sqrt(snr);
    double ber = 0.5 * erfc(z);
    EV << "bpsk snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double NistErrorRateModel::GetQpskBer(double snr) const
{
    double z = std::sqrt(snr / 2.0);
    double ber = 0.5 * erfc(z);
    EV << "qpsk snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double NistErrorRateModel::Get16QamBer(double snr) const
{
    double z = std::sqrt(snr / (5.0 * 2.0));
    double ber = 0.75 * 0.5 * erfc(z);
    EV << "QAM" << " snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double NistErrorRateModel::Get64QamBer(double snr) const
{
    double z = std::sqrt(snr / (21.0 * 2.0));
    double ber = 7.0 / 12.0 * 0.5 * erfc(z);
    EV << "64-Qam" << " snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double NistErrorRateModel::GetFecBpskBer(double snr, double nbits, uint32_t bValue) const
{
    double ber = GetBpskBer(snr);
    if (ber == 0.0)
    {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, nbits);
    return pms;
}

double NistErrorRateModel::GetFecQpskBer(double snr, double nbits, uint32_t bValue) const
{
    double ber = GetQpskBer(snr);
    if (ber == 0.0)
    {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, nbits);
    return pms;
}

double NistErrorRateModel::CalculatePe(double p, uint32_t bValue) const
{
    double D = std::sqrt(4.0 * p * (1.0 - p));
    double pe = 1.0;
    if (bValue == 1)
    {
        // code rate 1/2, use table 3.1.1
        // code rate 1/2, use table 3.1.1
        pe = 0.5 * ( 36.0 * std::pow (D, 10.0)
            + 211.0 * std::pow (D, 12.0)
            + 1404.0 * std::pow (D, 14.0)
            + 11633.0 * std::pow (D, 16.0)
            + 77433.0 * std::pow (D, 18.0)
            + 502690.0 * std::pow (D, 20.0)
            + 3322763.0 * std::pow (D, 22.0)
            + 21292910.0 * std::pow (D, 24.0)
            + 134365911.0 * std::pow (D, 26.0)
          );
    }
    else if (bValue == 2)
    {
        // code rate 2/3, use table 3.1.2
        // code rate 2/3, use table 3.1.2
        pe = 1.0 / (2.0 * bValue) *
          ( 3.0 * std::pow (D, 6.0)
            + 70.0 * std::pow (D, 7.0)
            + 285.0 * std::pow (D, 8.0)
            + 1276.0 * std::pow (D, 9.0)
            + 6160.0 * std::pow (D, 10.0)
            + 27128.0 * std::pow (D, 11.0)
            + 117019.0 * std::pow (D, 12.0)
            + 498860.0 * std::pow (D, 13.0)
            + 2103891.0 * std::pow (D, 14.0)
            + 8784123.0 * std::pow (D, 15.0)
          );
    }
    else if (bValue == 3)
    {
        // code rate 3/4, use table 3.1.2
        pe = 1.0 / (2.0 * bValue) *
          ( 42.0 * std::pow (D, 5.0)
            + 201.0 * std::pow (D, 6.0)
            + 1492.0 * std::pow (D, 7.0)
            + 10469.0 * std::pow (D, 8.0)
            + 62935.0 * std::pow (D, 9.0)
            + 379644.0 * std::pow (D, 10.0)
            + 2253373.0 * std::pow (D, 11.0)
            + 13073811.0 * std::pow (D, 12.0)
            + 75152755.0 * std::pow (D, 13.0)
            + 428005675.0 * std::pow (D, 14.0)
          );
    }
    else if (bValue == 5)
    {
        //802.11n support by S. Deronne
        pe = 1.0 / (2.0 * bValue) *
          ( 92.0 * std::pow (D, 4.0)
            + 528.0 * std::pow (D, 5.0)
            + 8694.0 * std::pow (D, 6.0)
            + 79453.0 * std::pow (D, 7.0)
            + 792114.0 * std::pow (D, 8.0)
            + 7375573.0 * std::pow (D, 9.0)
            + 67884974.0 * std::pow (D, 10.0)
            + 610875423.0 * std::pow (D, 11.0)
            + 5427275376.0 * std::pow (D, 12.0)
            + 47664215639.0 * std::pow (D, 13.0)
          );
    }
    else
    {
        ASSERT(false);
    }
    return pe;
}

double NistErrorRateModel::GetFec16QamBer(double snr, uint32_t nbits, uint32_t bValue) const
{
    double ber = Get16QamBer(snr);
    if (ber == 0.0)
    {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, (double)nbits);
    return pms;
}
double NistErrorRateModel::GetFec64QamBer(double snr, uint32_t nbits, uint32_t bValue) const
{
    double ber = Get64QamBer(snr);
    if (ber == 0.0)
    {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, (double)nbits);
    return pms;
}

double NistErrorRateModel::GetChunkSuccessRate(ModulationType mode, double snr, uint32_t nbits) const
{
    if (mode.getModulationClass() == MOD_CLASS_ERP_OFDM || mode.getModulationClass() == MOD_CLASS_OFDM || mode.getModulationClass() == MOD_CLASS_HT)
    {
        if (mode.getConstellationSize() == 2)
        {
            if (mode.getCodeRate() == CODE_RATE_1_2)
            {
                return GetFecBpskBer(snr,
                                     nbits,
                                     1 // b value
                                     );
            }
            else
            {
                return GetFecBpskBer(snr,
                                     nbits,
                                     3 // b value
                                     );
            }
        }
        else if (mode.getConstellationSize() == 4)
        {
            if (mode.getCodeRate() == CODE_RATE_1_2)
            {
                return GetFecQpskBer(snr,
                                     nbits,
                                     1 // b value
                                     );
            }
            else
            {
                return GetFecQpskBer(snr,
                                     nbits,
                                     3 // b value
                                     );
            }
        }
        else if (mode.getConstellationSize() == 16)
        {
            if (mode.getCodeRate() == CODE_RATE_1_2)
            {
                return GetFec16QamBer(snr,
                                      nbits,
                                      1 // b value
                                      );
            }
            else
            {
                return GetFec16QamBer(snr,
                                      nbits,
                                      3 // b value
                                      );
            }
        }
        else if (mode.getConstellationSize() == 64)
        {
            if (mode.getCodeRate() == CODE_RATE_2_3)
            {
                return GetFec64QamBer(snr,
                                      nbits,
                                      2 // b value
                                      );
            }
            else if (mode.getCodeRate () == CODE_RATE_5_6) //802.11n support by S. Deronne
            {
                return GetFec64QamBer (snr,
                                       nbits,
                                       5 // b value
                                       );
            }
            else
            {
                return GetFec64QamBer (snr,
                                       nbits,
                                       3 // b value
                                      );
            }
        }
    }
    else if (mode.getModulationClass() == MOD_CLASS_DSSS)
    {
        switch (mode.getDataRate())
        {
            case 1000000:
                return DsssErrorRateModel::GetDsssDbpskSuccessRate(snr, nbits);
            case 2000000:
                return DsssErrorRateModel::GetDsssDqpskSuccessRate(snr, nbits);
            case 5500000:
                return DsssErrorRateModel::GetDsssDqpskCck5_5SuccessRate(snr, nbits);
            case 11000000:
                return DsssErrorRateModel::GetDsssDqpskCck11SuccessRate(snr, nbits);
        }
    }


    return 0;
}

