/*

Copyright Ⓒ 2025 Andy Styles
All Rights Reserved

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "SessionPreviewWidget.h"
#include "HostAddressUtils.h"

SessionPreviewWidget::SessionPreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    clear();

    previewSecure->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    previewVerifyCert->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void SessionPreviewWidget::setSession(const Session &session)
{
    groupBox->setEnabled(true);

    previewAddress->setText(HostAddressUtils::format(session.hostName, session.hostPort, session.hostLU));
    previewModel->setText(session.terminalModel);
    previewSecure->setChecked(session.secureConnection);
    previewVerifyCert->setChecked(session.verifyCertificate);
}

void SessionPreviewWidget::clear()
{
    groupBox->setEnabled(false);

    previewAddress->clear();
    previewModel->clear();
    previewSecure->setChecked(false);
    previewVerifyCert->setChecked(false);
}
