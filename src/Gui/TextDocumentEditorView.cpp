/***************************************************************************
 *   Copyright (c) 2017 Markus Hovorka <m.hovorka@live.de>                 *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QPushButton>
#include <QString>


#include "TextDocumentEditorView.h"
#include "Application.h"
#include "BitmapFactory.h"
#include "Document.h"
#include "MainWindow.h"


using namespace Gui;

TYPESYSTEM_SOURCE_ABSTRACT(Gui::TextDocumentEditorView, Gui::MDIView)  // NOLINT

TextDocumentEditorView::TextDocumentEditorView(App::TextDocument* txtDoc, QPlainTextEdit* e, QWidget* parent)
    : MDIView(Application::Instance->getDocument(txtDoc->getDocument()), parent)
    , editor {e}
    , textDocument {txtDoc}
{
    setupEditor();
    setupConnection();
    setCentralWidget(editor);
    setWindowIcon(Gui::BitmapFactory().iconFromTheme("TextDocument"));

    // clang-format off
    // update editor actions on request
    Gui::MainWindow* mw = Gui::getMainWindow();
    connect(editor, &QPlainTextEdit::undoAvailable, mw, &MainWindow::updateEditorActions);
    connect(editor, &QPlainTextEdit::redoAvailable, mw, &MainWindow::updateEditorActions);
    connect(editor, &QPlainTextEdit::copyAvailable, mw, &MainWindow::updateEditorActions);
    connect(editor, &QPlainTextEdit::textChanged, this, &TextDocumentEditorView::textChanged);
    // clang-format on
}

TextDocumentEditorView::~TextDocumentEditorView()
{
    textConnection.disconnect();
    labelConnection.disconnect();
}

void TextDocumentEditorView::showEvent(QShowEvent* event)
{
    Gui::MainWindow* mw = Gui::getMainWindow();
    mw->updateEditorActions();
    MDIView::showEvent(event);
}

void TextDocumentEditorView::closeEvent(QCloseEvent* event)
{
    MDIView::closeEvent(event);
    if (event->isAccepted()) {
        aboutToClose = true;
        Gui::MainWindow* mw = Gui::getMainWindow();
        mw->updateEditorActions();
    }
}

void TextDocumentEditorView::setupEditor()
{
    // clang-format off
    connect(getEditor()->document(), &QTextDocument::modificationChanged,
            this, &TextDocumentEditorView::setWindowModified);
    // clang-format on
    labelChanged();
    refresh();
}

void TextDocumentEditorView::setupConnection()
{
    // NOLINTBEGIN
    textConnection = textDocument->connectText(std::bind(&TextDocumentEditorView::sourceChanged, this));
    labelConnection = textDocument->connectLabel(
        std::bind(&TextDocumentEditorView::labelChanged, this)
    );
    // NOLINTEND
}

void TextDocumentEditorView::sourceChanged()
{
    refresh();
}

void TextDocumentEditorView::textChanged()
{
    saveToObject();
}

void TextDocumentEditorView::labelChanged()
{
    setWindowTitle(QString::fromUtf8(textDocument->Label.getValue()) + QStringLiteral("[*]"));
}

void TextDocumentEditorView::refresh()
{
    QString text = QString::fromUtf8(textDocument->Text.getValue());
    getEditor()->setPlainText(text);
}

void TextDocumentEditorView::saveToObject()
{
    fastsignals::shared_connection_block textBlock {textConnection};
    textDocument->Text.setValue(getEditor()->document()->toPlainText().toUtf8());
    textDocument->purgeTouched();
}

QStringList TextDocumentEditorView::undoActions() const
{
    QStringList undo;
    undo << tr("Edit text");
    return undo;
}

QStringList TextDocumentEditorView::redoActions() const
{
    QStringList redo;
    redo << tr("Edit text");
    return redo;
}

bool TextDocumentEditorView::onHasMsg(Message msg) const
{
    // don't allow any actions if the editor is being closed
    if (aboutToClose) {
        return false;
    }

    switch (msg) {
        case Message::Save:
            return true;
        case Message::Cut:
            return !getEditor()->isReadOnly() && getEditor()->textCursor().hasSelection();
        case Message::Copy:
            return getEditor()->textCursor().hasSelection();
        case Message::Paste:
            if (getEditor()->isReadOnly()) {
                return false;
            }
            return !QApplication::clipboard()->text().isEmpty();
        case Message::Undo:
            return getEditor()->document()->isUndoAvailable();
        case Message::Redo:
            return getEditor()->document()->isRedoAvailable();
        default:
            return false;
    }
}

bool TextDocumentEditorView::onMsg(Message msg, const char** output)
{
    Q_UNUSED(output)

    // don't allow any actions if the editor is being closed
    if (aboutToClose) {
        return false;
    }

    switch (msg) {
        case Message::Save:
            saveToObject();
            getGuiDocument()->save();
            return true;
        case Message::Cut:
            getEditor()->cut();
            return true;
        case Message::Copy:
            getEditor()->copy();
            return true;
        case Message::Paste:
            getEditor()->paste();
            return true;
        case Message::Undo:
            getEditor()->undo();
            return true;
        case Message::Redo:
            getEditor()->redo();
            return true;
        default:
            return false;
    }
}

#include "moc_TextDocumentEditorView.cpp"
