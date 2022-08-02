/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "vsss_secp256k1.h"

using std::vector;
using safeheron::bignum::BN;
using safeheron::curve::CurvePoint;
using safeheron::curve::Curve;
using safeheron::curve::CurveType;

namespace safeheron{
namespace sss {
namespace vsss_secp256k1 {

static const Curve *curv = GetCurveParam(CurveType::SECP256K1);

void MakeShares(vector<Point> &shares, const BN &secret, int threshold, const vector<BN> &shareIndexs) {
    vsss::MakeShares(shares, secret, threshold, shareIndexs, curv->n);
}

void MakeSharesWithCommits(vector<Point> &shares, vector<CurvePoint> &commits, const BN &secret,
                                          int threshold, const vector<BN> &shareIndexs) {
    vsss::MakeSharesWithCommits(shares, commits, secret, threshold, shareIndexs, curv->n, curv->g);
}

void MakeSharesWithCommits(vector<Point> &shares, vector<CurvePoint> &commits, const BN &secret,
                                          int threshold, int num) {
    vector<BN> shareIndexs;
    for(int i = 1; i <= num; i ++){
        shareIndexs.push_back(BN(i));
    }
    vsss::MakeSharesWithCommits(shares, commits, secret, threshold, shareIndexs, curv->n, curv->g);
}

void MakeSharesWithCommitsAndCoes(vector<Point> &shares, vector<CurvePoint> &commits, const BN &secret,
                                                 int threshold, const vector<BN> &shareIndexs,
                                                 const vector<BN> &coeArray) {
    vsss::MakeSharesWithCommitsAndCoes(shares, commits, secret, threshold, shareIndexs, coeArray, curv->n, curv->g);
}

bool VerifyShare(const vector<CurvePoint> &commits, const BN &shareIndex, const BN &share) {
    return vsss::VerifyShare(commits, shareIndex, share, curv->g, curv->n);
}

void RecoverSecret(BN &secret, const vector<Point> &shares) {
    vsss::RecoverSecret(secret, shares, curv->n);
}

}
}
}
