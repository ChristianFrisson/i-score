#!/bin/sh

echo 'Resources'
echo '...Ajout'
rm -rf acousmoscribe.app/Contents/Resources/images acousmoscribe.app/Contents/Resources/documentation
rm -rf acousmoscribe.app/Contents/Resources/images acousmoscribe.app/Contents/Resources/images
rm -rf acousmoscribe.app/Contents/Resources/translations acousmoscribe.app/Contents/Resources/translations
mkdir acousmoscribe.app/Contents/Resources/images
mkdir acousmoscribe.app/Contents/Resources/documentation
mkdir acousmoscribe.app/Contents/Resources/translations

echo '...Transfert'
cp -R images/ acousmoscribe.app/Contents/Resources/images/
cp -R documentation/ acousmoscribe.app/Contents/Resources/documentation/
cp -R translations/ acousmoscribe.app/Contents/Resources/translations/


