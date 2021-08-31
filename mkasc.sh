#!/bin/sh

echo "Calculating SHA256 for all files in /transfer - firetools version $1"

cd /transfer
sha256sum -- * > "firetools-$1-unsigned"
gpg --clearsign --digest-algo SHA256 < "firetools-$1-unsigned" > "firetools-$1.asc"
gpg --verify "firetools-$1.asc"
gpg --detach-sign --armor "firetools-$1.tar.xz"
rm "firetools-$1-unsigned"
