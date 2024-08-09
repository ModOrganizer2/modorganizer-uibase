import json
from pathlib import Path

from PyQt6.lupdate.source_file import SourceFile
from PyQt6.lupdate.translation_file import TranslationFile
from PyQt6.lupdate.translations import Context, Message

folder = Path(__file__).parent

tr_files = [
    TranslationFile(path, no_obsolete=False, no_summary=False, verbose=True)
    for path in folder.glob("*.ts")
]

for metadata_path in folder.parent.joinpath("data", "extensions").glob("**/*.json"):
    with open(metadata_path, "rb") as fp:
        metadata = json.load(fp)

    if "translation-context" not in metadata:
        continue

    source = SourceFile(filename=metadata_path)

    context = Context(name=metadata["translation-context"])

    for key in ("name", "description"):
        if key not in metadata:
            continue

        context.messages.append(
            Message(
                filename=metadata_path,
                line_nr=-1,
                source=metadata[key],
                comment=None,
                numerus=None,
            )
        )

    source.contexts.append(context)

    for tr_file in tr_files:
        tr_file.update(source)

for tr_file in tr_files:
    tr_file.write()
