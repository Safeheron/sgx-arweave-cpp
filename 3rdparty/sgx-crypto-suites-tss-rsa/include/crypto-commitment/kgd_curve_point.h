/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef CPP_MPC_KGD_CURVE_POINT_H
#define CPP_MPC_KGD_CURVE_POINT_H

#include "crypto-bn/bn.h"
#include "crypto-curve/curve.h"
#include "commitment.pb.h"

namespace safeheron{
namespace commitment {

class KgdCurvePoint {
public:
    safeheron::curve::CurvePoint point_;
    safeheron::bignum::BN blind_factor_;

    KgdCurvePoint() {}
    KgdCurvePoint(safeheron::curve::CurvePoint point, safeheron::bignum::BN blind_factor) {
        point_ = point;
        blind_factor_ = blind_factor;
    }

public:
    bool ToProtoObject(safeheron::proto::KGD &kgdNum) const;
    bool FromProtoObject(const safeheron::proto::KGD &party);

    bool ToBase64(std::string& base64) const;
    bool FromBase64(const std::string& base64);

    bool ToJsonString(std::string &json_str) const;
    bool FromJsonString(const std::string &json_str);
};

}
}

#endif //CPP_MPC_KGD_CURVE_POINT_H
