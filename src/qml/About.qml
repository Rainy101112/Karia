import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.coreaddons

FormCard.AboutPage {
    FormCard.FormHeader {
        title: "hello"
    }

    FormCard.FormCard {
        FormCard.FromTextDelegate {
            text: "hello"
            description: "hello"
        }
    }
}
